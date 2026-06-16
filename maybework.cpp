/**
 * Mecanum Robot - Master Small Controller
 * ESP32 Dual-Core | PS4 Controller | I2C to slave
 *
 * Core 0 → TaskGamepad  : อ่านจอย + ส่ง I2C + Auto-reconnect
 * Core 1 → TaskMotor    : Ramp + Kinematics + ขับมอเตอร์
 *
 * Auto-reconnect:
 *   PS4Controller library จัดการ BT ระดับล่างเองอยู่แล้ว
 *   เราเพิ่ม watchdog ตรวจ isConnected() และ log สถานะ
 *   เมื่อหลุด → หยุดมอเตอร์ทันที + รอ reconnect อัตโนมัติ
 *   ไม่ต้อง restart ESP32 หรือกดปุ่มใดๆ
 */

#include "Arduino.h"
#include "Motor.h"
#include "config.h"
#include "Kinematics.h"
#include <PS4Controller.h>
#include <Wire.h>

// ─────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────
#define I2C_SLAVE_ADDR  0x04
#define STICK_DEADZONE  100     // Analog stick deadzone (0–127)
#define COMMAND_RATE    50      // Hz

// Speed presets (fast / slow mode เมื่อกด R2)
static const float WALK_FAST  = 1.4f, WALK_SLOW  = 0.5f;
static const float TURN_FAST  = 3.2f, TURN_SLOW  = 2.0f;
static const float SLIDE_FAST = 2.0f, SLIDE_SLOW = 1.2f;

// Ramp: หน่วย/วินาที — เพิ่มเพื่อให้ตอบสนองเร็วขึ้น, ลดเพื่อนุ่มนวลขึ้น
static const float RAMP_RATE = 4.0f;
static const float MOTOR_DT  = 1.0f / COMMAND_RATE;

// ─────────────────────────────────────────────
//  Task Handles
// ─────────────────────────────────────────────
static TaskHandle_t TaskGamepadHandle;
static TaskHandle_t TaskMotorHandle;

// ─────────────────────────────────────────────
//  Motor & Kinematics
// ─────────────────────────────────────────────
Motor MotorFL(MotorPinFLM1_A, MotorPinFLM1_B, M_MAX_RPM);
Motor MotorFR(MotorPinFRM1_A, MotorPinFRM1_B, M_MAX_RPM);
Motor MotorRL(MotorPinRLM1_A, MotorPinRLM1_B, M_MAX_RPM);
Motor MotorRR(MotorPinRRM1_A, MotorPinRRM1_B, M_MAX_RPM);

Kinematics kinematics(
    Kinematics::MECANUM, M_MAX_RPM,
    WHEEL_DIAMETER, FR_WHEELS_DISTANCE, LR_WHEELS_DISTANCE
);

// ─────────────────────────────────────────────
//  Shared State — ป้องกัน Race Condition
// ─────────────────────────────────────────────
static portMUX_TYPE g_mux = portMUX_INITIALIZER_UNLOCKED;
static volatile float g_req_x       = 0.0f;
static volatile float g_req_y       = 0.0f;
static volatile float g_req_z       = 0.0f;
static volatile bool  g_ps4_online  = false;  // true = จอยต่ออยู่

// ─────────────────────────────────────────────
//  I2C Helper
// ─────────────────────────────────────────────
static void sendI2C(char cmd) {
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write(cmd);
    uint8_t err = Wire.endTransmission();
    if (err != 0) {
        Serial.printf("[I2C] Error %u on cmd '%c'\n", err, cmd);
    }
}

// ─────────────────────────────────────────────
//  Ramp Helper (clamp-based — rate คงที่แน่นอน)
// ─────────────────────────────────────────────
static inline float rampToward(float cur, float target, float step) {
    float diff = target - cur;
    if      (diff >  step) return cur + step;
    else if (diff < -step) return cur - step;
    else                   return target;
}

// ─────────────────────────────────────────────
//  Task 0 : Gamepad — Core 0
//  Auto-reconnect: library จัดการ BT เอง
//  เราแค่ตรวจ isConnected() และจัดการ state transition
// ─────────────────────────────────────────────
void TaskGamepad(void *pvParameters) {
    // ── Lift / I2C state ──
    char last_lift = 'S';

    // ── Button edge-detect state ──
    bool sq_prev       = false;  // Square  → I2C 'A'
    bool ci_prev       = false;  // Circle  → I2C 'B'
    bool tri_prev      = false;  // Triangle → toggle Relay3
    bool share_prev    = false;  // Share    → toggle Relay1
    bool options_prev  = false;  // Options  → toggle Relay4

    bool was_connected = false;

    for (;;) {
        bool now_connected = PS4.isConnected();

        // ── Disconnected → Connected ──
        if (now_connected && !was_connected) {
            Serial.println("[PS4] Connected!");
            // reset ทุก edge เพื่อไม่ให้ปุ่มค้าง
            sq_prev = ci_prev = tri_prev = share_prev = options_prev = true;
            last_lift     = 'S';
            was_connected = true;

            portENTER_CRITICAL(&g_mux);
            g_ps4_online = true;
            portEXIT_CRITICAL(&g_mux);
        }

        // ── Connected → Disconnected ──
        if (!now_connected && was_connected) {
            Serial.println("[PS4] Disconnected! Waiting to reconnect...");
            was_connected = false;

            portENTER_CRITICAL(&g_mux);
            g_req_x = g_req_y = g_req_z = 0.0f;
            g_ps4_online = false;
            portEXIT_CRITICAL(&g_mux);

            sendI2C('S');   // หยุด Lift ที่ slave
            last_lift = 'S';

            // Cross ค้าง → ปล่อย Relay2
            digitalWrite(RelayM1_PIN2, HIGH);
        }

        // ── ทำงานปกติ ──
        if (now_connected) {
            bool slow   = PS4.R2();
            float walk  = slow ? WALK_SLOW  : WALK_FAST;
            float turn  = slow ? TURN_SLOW  : TURN_FAST;
            float slide = slow ? SLIDE_SLOW : SLIDE_FAST;

            // D-Pad → เคลื่อนที่
            float tx = 0.0f, ty = 0.0f, tz = 0.0f;
            if      (PS4.Up())    tx =  walk;
            else if (PS4.Down())  tx = -walk;
            if      (PS4.Left())  ty =  slide;
            else if (PS4.Right()) ty = -slide;
            if      (PS4.L1())    tz =  turn;
            else if (PS4.R1())    tz = -turn;

            portENTER_CRITICAL(&g_mux);
            g_req_x = tx; g_req_y = ty; g_req_z = tz;
            portEXIT_CRITICAL(&g_mux);

            // ── Relay Control ──
            // Triangle (Rising Edge) → toggle Relay3
            bool tri_now = PS4.Triangle();
            if (tri_now && !tri_prev)
                digitalWrite(RelayM1_PIN3, !digitalRead(RelayM1_PIN3));
            tri_prev = tri_now;

            // Cross (Hold) → Relay2 LOW ขณะกดอยู่
            digitalWrite(RelayM1_PIN2, PS4.Cross() ? LOW : HIGH);

            // Share (Rising Edge) → toggle Relay1
            bool share_now = PS4.Share();
            if (share_now && !share_prev)
                digitalWrite(RelayM1_PIN1, !digitalRead(RelayM1_PIN1));
            share_prev = share_now;

            // Options (Rising Edge) → toggle Relay4
            bool opt_now = PS4.Options();
            if (opt_now && !options_prev)
                digitalWrite(RelayM1_PIN4, !digitalRead(RelayM1_PIN4));
            options_prev = opt_now;

            // ── I2C Button → Slave ──
            bool sq_now = PS4.Square(), ci_now = PS4.Circle();
            if (sq_now && !sq_prev) sendI2C('A');
            if (ci_now && !ci_prev) sendI2C('B');
            sq_prev = sq_now;
            ci_prev = ci_now;

            // ── Lift (Right Stick Y + Left Stick Y) ──
            int ry = PS4.RStickY();
            int ly = PS4.LStickY();
            char lift_cmd = last_lift;

            if (abs(ry) > STICK_DEADZONE) {
                // Right Stick → Up/Down (U/D)
                lift_cmd = (ry > 0) ? (slow ? 'd' : 'D')
                                    : (slow ? 'u' : 'U');
            } else if (abs(ly) > STICK_DEADZONE) {
                // Left Stick → Open/Close (O/C)
                lift_cmd = (ly > 0) ? (slow ? 'o' : 'O')
                                    : (slow ? 'c' : 'C');
            } else {
                // ปล่อย Stick → หยุดเฉพาะ U/D (ไม่หยุด O/C)
                if (last_lift == 'U' || last_lift == 'u' ||
                    last_lift == 'D' || last_lift == 'd') {
                    lift_cmd = 'S';
                }
            }

            if (lift_cmd != last_lift) {
                sendI2C(lift_cmd);
                last_lift = lift_cmd;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// ─────────────────────────────────────────────
//  Task 1 : Motor Control — Core 1
// ─────────────────────────────────────────────
void TaskMotor(void *pvParameters) {
    float cur_x = 0.0f, cur_y = 0.0f, cur_z = 0.0f;
    const float step = RAMP_RATE * MOTOR_DT;

    for (;;) {
        float req_x, req_y, req_z;
        bool  online;

        portENTER_CRITICAL(&g_mux);
        req_x  = g_req_x;
        req_y  = g_req_y;
        req_z  = g_req_z;
        online = g_ps4_online;
        portEXIT_CRITICAL(&g_mux);

        if (!online) {
            // จอยหลุด → หยุดมอเตอร์ทันที ไม่ Ramp
            cur_x = cur_y = cur_z = 0.0f;
            MotorFL.run(0); MotorFR.run(0);
            MotorRL.run(0); MotorRR.run(0);
        } else {
            // Ramp ปกติ
            cur_x = rampToward(cur_x, req_x, step);
            cur_y = rampToward(cur_y, req_y, step);
            cur_z = rampToward(cur_z, req_z, step);

            Kinematics::rpm r = kinematics.getRPM(cur_x, cur_y, cur_z);
            MotorFL.runRPM(r.motor1);
            MotorFR.runRPM(r.motor2);
            MotorRL.runRPM(r.motor3);
            MotorRR.runRPM(r.motor4);
        }

        vTaskDelay(pdMS_TO_TICKS(1000 / COMMAND_RATE));
    }
}

// ─────────────────────────────────────────────
//  Setup
// ─────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    setCpuFrequencyMhz(240);

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);

    PS4.begin("08:a6:f7:10:a8:5c");

    pinMode(RelayM1_PIN1, OUTPUT);
    pinMode(RelayM1_PIN2, OUTPUT);
    pinMode(RelayM1_PIN3, OUTPUT);
    pinMode(RelayM1_PIN4, OUTPUT);

    // Relay เริ่มต้น HIGH (ปิด)
    digitalWrite(RelayM1_PIN1, HIGH);
    digitalWrite(RelayM1_PIN2, HIGH);
    digitalWrite(RelayM1_PIN3, HIGH);
    digitalWrite(RelayM1_PIN4, HIGH);

    Serial.println("[BOOT] Ready. Waiting for PS4 controller...");

    xTaskCreatePinnedToCore(TaskGamepad, "Gamepad", 8192, NULL, 1, &TaskGamepadHandle, 0);
    xTaskCreatePinnedToCore(TaskMotor,  "Motor",   4096, NULL, 2, &TaskMotorHandle,   1);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(5000));
}
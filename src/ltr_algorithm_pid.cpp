#include "ltr_main.h"

float g_turn_kp = 20.0;
float g_turn_ki = 0.0;
float g_turn_kd = 5.0;
float g_turn_p  = 0.0;
float g_turn_i  = 0.0;
float g_turn_d  = 0.0;

float g_turn_error_k0   = 2.0;
float g_turn_error_k1   = 1.0;
float g_turn_error_last = 0.0;

uint32_t g_timer_turn_pid = 0;

void cacluteTurnPID(void) {
    uint32_t ms = millis();
    if (ms - g_timer_turn_pid >= 50) {
#ifdef DEBUG_PID_CYCLE
        printf("Time turn: %lums\n", ms);
#endif
        // 获取每个红外反射传感器的模拟值
        // Get the analog value of each infrared reflection sensor
        uint16_t a0 = analogRead(A0);
        uint16_t a1 = analogRead(A1);
        uint16_t a2 = analogRead(A2);
        uint16_t a3 = analogRead(A3);
#ifdef DEBUG_ADC_VALUE
        printf("A0: %u, A1: %u, A2: %u, A3: %u\n", a0, a1, a2, a3);
#endif
        // 使用加权求和的方式来表示系统误差
        // Use weighted summation to represent systematic errors
        float turn_error = 0.0;
        turn_error = turn_error + g_turn_error_k0 * a0;
        turn_error = turn_error + g_turn_error_k1 * a1;
        turn_error = turn_error - g_turn_error_k1 * a2;
        turn_error = turn_error - g_turn_error_k0 * a3;

        // 计算转向环PID
        // Calculate turn PID
        g_turn_p = g_turn_kp * turn_error;
        g_turn_i = g_turn_ki * (g_turn_i + turn_error);
        g_turn_d = g_turn_kd * (turn_error - g_turn_error_last);
        float turn_sum = (g_turn_p + g_turn_i + g_turn_d) / 100;

#ifdef DEBUG_PID_VALUE
        printf("Turn E: %.2f, Turn P: %.2f, Turn I: %.2f, Turn D: %.2f, "
               "Turn Sum: %.2f\n",
                turn_error, g_turn_p, g_turn_i, g_turn_d, turn_sum);
#endif

        // 根据计算结果与当前所处状态控制电机的转速或转向
        // Control the speed or direction of the motor according to the
        // calculation result and the current state
        int16_t motor_speed_a = MOTOR_SPEED_REF;
        int16_t motor_speed_b = MOTOR_SPEED_REF;

        // 规定机器人水平向左为负向右为正，此时如果PID计算结果为正，则机器人需要向右偏移，
        // 右侧电机应降低一定的速度，速度变化值由PID计算结果的绝对值表示，反方向同理
        // It is stipulated that the horizontal left of the robot is negative
        // and the right is positive. At this time, if the PID calculation
        // result is positive, the robot needs to shift to the right, and the
        // right motor should reduce a certain speed. The speed change value is
        // represented by the absolute value of the PID calculation result.
        // The same goes in the opposite direction
        if (turn_sum > 0) {
            motor_speed_b = motor_speed_b - turn_sum;
        }
        else {
            motor_speed_a = motor_speed_a - abs(turn_sum);
        }

#ifdef DEBUG_MTR_VALUE
        printf("Motor A: %d, Motor B: %d\n\n", motor_speed_a, motor_speed_b);
#endif

        setMotorSpeed(MOTOR_A, motor_speed_a);
        setMotorSpeed(MOTOR_B, motor_speed_b);

        g_turn_error_last = turn_error;
        g_timer_turn_pid = ms;
    }
}

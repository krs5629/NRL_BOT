#include "NRL.h"

// Two software motor objects.
// Each one represents an entire side of the drivetrain.
//
// LEFT side: 2 physical motors
// RIGHT side: 2 physical motors

static HexaDCMotor leftMotor({
    .dirPin = MOTOR_L_DIR,
    .pwmPin = MOTOR_L_PWM,
    .flipped = true
});

static HexaDCMotor rightMotor({
    .dirPin = MOTOR_R_DIR,
    .pwmPin = MOTOR_R_PWM
});

// TankDrive treats the two motors as the left and right sides.
static TankDrive drive(leftMotor, rightMotor);


class StudentTeleOp : public NRLOpMode {
public:

    void init() override {
        leftMotor.begin();
        rightMotor.begin();

        // Smooth acceleration/deceleration.
        drive.setRampRate(0.1f);
    }

    void loop() override {
        // RIGHT JOYSTICK ONLY
        //
        // Right Y = forward/backward
        // Right X = left/right turning
        drive.drive(gamepad1.rightY(), gamepad1.rightX());
    }

    void stop() override {
        drive.stop();
    }
};


REGISTER_OPMODE(StudentTeleOp, "Student TeleOp", TELEOP);

#include "NRL.h"

// Left drivetrain side
// Represents the two physical motors on the left side.
static HexaDCMotor leftMotor({
    .dirPin = MOTOR_L_DIR,
    .pwmPin = MOTOR_L_PWM,
    .flipped = true
});

// Right drivetrain side
// Represents the two physical motors on the right side.
static HexaDCMotor rightMotor({
    .dirPin = MOTOR_R_DIR,
    .pwmPin = MOTOR_R_PWM
});

// NRL's drivetrain controller.
// It combines forward and turning commands into
// left/right motor commands.
static TankDrive drive(leftMotor, rightMotor);


class StudentTeleOp : public NRLOpMode {
public:

    void init() override {
        leftMotor.begin();
        rightMotor.begin();

        // Smoothly ramps motor commands instead of
        // changing motor power instantaneously.
        drive.setRampRate(0.1f);
    }

    void loop() override {
        // RIGHT JOYSTICK:
        //
        // rightY() = forward / reverse
        // rightX() = left / right turning
        //
        // TankDrive converts these into left/right
        // drivetrain commands.
        drive.drive(
            gamepad1.rightY(),
            gamepad1.rightX()
        );
    }

    void stop() override {
        drive.stop();
    }
};


REGISTER_OPMODE(StudentTeleOp, "Student TeleOp", TELEOP);

#include "NRL.h"


// ============================================================
// DRIVE MOTORS
// ============================================================

static HexaDCMotor leftMotor({
    .dirPin = MOTOR_L_DIR,
    .pwmPin = MOTOR_L_PWM,
    .flipped = true
});

static HexaDCMotor rightMotor({
    .dirPin = MOTOR_R_DIR,
    .pwmPin = MOTOR_R_PWM
});

static TankDrive drive(leftMotor, rightMotor);


// ============================================================
// ARM SERVO
// ============================================================

static HexaServo arm({
    .signalPin = SERVO_1,
    .startAngle = 90.0f,
    .minAngle = 20.0f,
    .maxAngle = 160.0f,
    .offsetDeg = 0.0f,
    .settleMs = 0
});

static constexpr float ARM_SPEED_DEG_PER_SEC = 90.0f;
static constexpr float ARM_DEADBAND = 0.08f;

// false = use leftY() direction as-is
// true  = reverse the leftY() direction
static constexpr bool ARM_REVERSED = false;


// ============================================================
// CLAW SERVO
// ============================================================

static HexaServo claw({
    .signalPin = SERVO_2,
    .startAngle = 90.0f,
    .minAngle = 20.0f,
    .maxAngle = 160.0f,
    .offsetDeg = 0.0f,
    .settleMs = 0
});

static constexpr float CLAW_SPEED_DEG_PER_SEC = 45.0f;


// ============================================================
// STUDENT TELEOP
// ============================================================

class StudentTeleOp : public NRLOpMode {

public:

    void init() override {

        // ----------------------------------------------------
        // Configure servo ramp rates BEFORE begin().
        // begin() moves each servo to its startAngle.
        // ----------------------------------------------------

        arm.setRampRate(ARM_SPEED_DEG_PER_SEC);
        claw.setRampRate(CLAW_SPEED_DEG_PER_SEC);

        // ----------------------------------------------------
        // Initialize hardware
        // ----------------------------------------------------

        leftMotor.begin();
        rightMotor.begin();

        arm.begin();
        claw.begin();

        // ----------------------------------------------------
        // Drivetrain smoothing
        // ----------------------------------------------------

        drive.setRampRate(0.1f);
    }


    void loop() override {

        // ====================================================
        // WHEELS
        //
        // RIGHT JOYSTICK:
        //   Y = forward / reverse
        //   X = turning
        // ====================================================

        drive.drive(
            gamepad1.rightY(),
            gamepad1.rightX()
        );


        // ====================================================
        // ARM
        //
        // LEFT JOYSTICK Y = arm movement
        // ====================================================

        float armInput = gamepad1.leftY();


        // ----------------------------------------------------
        // DEADZONE
        //
        // Ignore tiny joystick movements around center.
        // ----------------------------------------------------

        if (fabs(armInput) < ARM_DEADBAND) {

            armInput = 0.0f;

        }
        else {

            // Rescale the remaining range so that the useful
            // joystick range still reaches approximately -1 to +1.

            if (armInput > 0.0f) {

                armInput =
                    (armInput - ARM_DEADBAND) /
                    (1.0f - ARM_DEADBAND);

            }
            else {

                armInput =
                    (armInput + ARM_DEADBAND) /
                    (1.0f - ARM_DEADBAND);
            }
        }


        // ----------------------------------------------------
        // ARM DIRECTION
        // ----------------------------------------------------

        if (ARM_REVERSED) {
            armInput = -armInput;
        }


        // ----------------------------------------------------
        // ARM MOVEMENT
        //
        // NRL TELEOP loop = 100 Hz.
        // 90 degrees/sec ÷ 100 = 0.9 degrees/loop.
        // ----------------------------------------------------

        constexpr float LOOP_HZ = 100.0f;

        const float armDegreesPerLoop =
            ARM_SPEED_DEG_PER_SEC / LOOP_HZ;


        if (armInput != 0.0f) {

            arm.moveBy(
                armInput * armDegreesPerLoop
            );
        }


        // ====================================================
        // CLAW
        //
        // RB HELD → CLOSE
        // LB HELD → OPEN
        //
        // Neither held → hold current position
        // Both held → do nothing
        // ====================================================

        if (gamepad1.pressed(BTN_RB) &&
            !gamepad1.pressed(BTN_LB)) {

            // RB = close claw
            claw.moveBy(
                CLAW_SPEED_DEG_PER_SEC / LOOP_HZ
            );

        }
        else if (gamepad1.pressed(BTN_LB) &&
                 !gamepad1.pressed(BTN_RB)) {

            // LB = open claw
            claw.moveBy(
                -CLAW_SPEED_DEG_PER_SEC / LOOP_HZ
            );
        }
    }


    void stop() override {

        drive.stop();
    }
};


// ============================================================
// REGISTER TELEOP
// ============================================================

REGISTER_OPMODE(
    StudentTeleOp,
    "Student TeleOp",
    TELEOP
);

/* # ⚠️ BEFORE YOU FLASH IT: CHECK THIS

### 1. Confirm the servo port

Your ZIP's `BoardPins.h` says:

```text
SERVO_1 = GPIO 39
SERVO_2 = GPIO 40
SERVO_3 = GPIO 41
SERVO_4 = GPIO 42
```

The code assumes:

> **Arm servo → SERVO_1**

So physically verify that.

If it's actually plugged into SERVO_2, change:

```cpp
.signalPin = SERVO_1,
```

to:

```cpp
.signalPin = SERVO_2,
```

Don't change anything else.

---

# Arm testing checklist

Do these **in order**.

## 🔧 A. Before powering the robot

* [ ] Power OFF.
* [ ] Check servo is plugged into the intended NRL servo port.
* [ ] Check servo connector orientation.
* [ ] Check signal/power/ground connections.
* [ ] Make sure servo horn is firmly attached.
* [ ] Make sure the arm linkage is mechanically secure.
* [ ] Move the arm carefully by hand with power OFF.
* [ ] Check that the arm isn't hitting the chassis.
* [ ] Check that no linkage is binding.
* [ ] Check that the servo isn't being forced against a hard stop.

---

## 🟢 B. First powered test

**Do not put the arm under a load yet.**

* [ ] Put the robot somewhere stable.
* [ ] Keep the arm clear of anything it could hit.
* [ ] Power the robot.
* [ ] Pair the controller.
* [ ] Select `Student TeleOp`.
* [ ] Press INIT.
* [ ] The servo should go toward **90°**.

### If it immediately goes somewhere crazy

**STOP.**

Don't keep testing it.

Check:

* [ ] Correct servo connector?
* [ ] Correct servo type?
* [ ] Signal/power/ground orientation?
* [ ] Is the servo horn installed in the expected position?
* [ ] Is the mechanical linkage already forcing the servo?

---

# 🕹️ C. Test the LEFT joystick

Start with **tiny movements**.

### Test 1

Push left joystick slightly UP.

Expected:

```text
LEFT STICK ↑
     ↓
ARM GOES UP
```

If that happens:

* [x] Direction correct.

### Test 2

Release the joystick.

Expected:

```text
LEFT STICK CENTER
       ↓
ARM STOPS
       ↓
ARM HOLDS POSITION
```

It should **not return to 90°**.

### Test 3

Push left joystick DOWN.

Expected:

```text
LEFT STICK ↓
     ↓
ARM GOES DOWN
```

---

# 🔄 If the arm moves in the opposite direction

For example:

```text
LEFT STICK ↑
     ↓
ARM GOES DOWN ❌
```

Don't change the servo wiring.

Change:

```cpp
static constexpr bool ARM_REVERSED = false;
```

to:

```cpp
static constexpr bool ARM_REVERSED = true;
```

Then flash again.

Expected:

```text
LEFT STICK ↑ → ARM UP
LEFT STICK ↓ → ARM DOWN
```

---

# 📐 If the arm only moves about 60°

This is important.

The code has:

```cpp
.minAngle = 20.0f,
.maxAngle = 160.0f,
```

and starts at:

```cpp
90°
```

So the **software-commanded range is 140°**, not 60°.

If you physically see only ~60° of movement:

### Check these in order:

* [ ] Is the arm hitting a mechanical stop?
* [ ] Is the servo horn/linkage installed at the wrong angle?
* [ ] Is the linkage geometry limiting the motion?
* [ ] Is the servo actually the expected 180° servo?
* [ ] Is the servo getting enough power?
* [ ] Does the servo make a buzzing/straining sound at the end of travel?
* [ ] Does it stop at the exact same physical position every time?

If it **hits a hard stop**, don't increase the software limits.

Instead, reduce the limits.

For example:

```cpp
.minAngle = 40.0f,
.maxAngle = 140.0f,
```

Then the usable range is 100°.

---

# 🐌 If the arm is too slow

Current setting:

```cpp
static constexpr float ARM_SPEED_DEG_PER_SEC = 90.0f;
```

Try:

```cpp
static constexpr float ARM_SPEED_DEG_PER_SEC = 120.0f;
```

Don't immediately jump to a huge value.

Test progressively:

```text
60  → slow
90  → recommended starting point
120 → faster
```

---

# 🐇 If the arm is too fast

Try:

```cpp
static constexpr float ARM_SPEED_DEG_PER_SEC = 60.0f;
```

For an NRL arm carrying something, I'd rather start **too slow than too fast** and increase it after testing.

---

# 😵 If the arm jitters around the center

Increase:

```cpp
static constexpr float ARM_DEADBAND = 0.08f;
```

to:

```cpp
static constexpr float ARM_DEADBAND = 0.10f;
```

or:

```cpp
static constexpr float ARM_DEADBAND = 0.12f;
```

Don't make the deadband enormous because then the center portion of the joystick won't respond.

---

# 🔌 If the arm doesn't move at all

Check:

* [ ] Servo plugged into correct port.
* [ ] Correct `SERVO_1`/`SERVO_2` setting.
* [ ] Servo has power.
* [ ] Ground connected.
* [ ] Signal wire oriented correctly.
* [ ] Robot firmware successfully compiled.
* [ ] Correct robot firmware was flashed.
* [ ] Controller is actually paired.
* [ ] `Student TeleOp` is selected.
* [ ] INIT was completed and the OpMode was started.
* [ ] Left joystick is actually producing input.
* [ ] Try the right joystick to confirm the controller itself works.

If the **wheels work but the arm doesn't**, that makes a controller/pairing problem less likely and points more toward the servo connection, servo port, servo power, or arm configuration.

---

# 🔥 If the servo gets hot / buzzes loudly / struggles

**Stop the test.**

That can mean the servo is continuously fighting a mechanical obstruction or trying to hold a load it can't handle.

Check:

* [ ] Arm isn't physically jammed.
* [ ] Servo isn't against its mechanical limit.
* [ ] Linkage isn't over-constrained.
* [ ] Arm isn't too heavy for the servo.
* [ ] Software limits aren't commanding into the mechanism.

Your ZIP's `HexaServo` configuration deliberately uses `settleMs = 0` by default for holding torque, so the servo **will keep holding its commanded position**. That's useful for an arm, but it also means a mechanically blocked arm can keep loading the servo.

---

# 🚗 D. Finally test wheels + arm together

Once the arm passes its individual checks:

### Test 1

```text
Right ↑
Left center
```

→ Bot drives forward, arm stays still.

### Test 2

```text
Right ↑
Left ↑
```

→ Bot drives forward + arm goes up.

### Test 3

```text
Right ←
Left ↓
```

→ Bot turns left + arm goes down.

### Test 4

Release both sticks.

→ Bot stops commanding movement and arm holds its last position.

### Test 5

Move both sticks simultaneously in different directions.

→ Verify neither system interferes with the other.

---

## One thing I would **not** change

Keep your existing:

```cpp
drive.setRampRate(0.1f);
```

Your ZIP's `TankDrive` specifically describes `0.1` as the smoothing value that spreads a full reversal over about 200 ms at the 100 Hz loop. That's a sensible starting point for the drivetrain and also reduces sudden current spikes that could affect the servo rail.

And **don't edit**:

```text
RobotFirmware/src/RobotMain.ino
RobotFirmware/lib/...
```

Your ZIP explicitly structures the project so your student code belongs in:

```text
RobotFirmware/opmodes/
```

So the only file you need to replace for this change is:

```text
RobotFirmware/opmodes/StudentTeleop.cpp
```

**One final hardware-specific thing:** this code assumes your arm has **one standard position servo** connected to `SERVO_1`. If your arm actually uses **two servos, a continuous-rotation servo, or a geared motor**, tell me that before flashing because the control code would need to be different
https://chatgpt.com/share/6ac53681-4430-83ee-8040-586ac5d8de03
https://chatgpt.com/share/6ac536e7-7768-83ec-8e7a-9360c0904d26
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <time.h>

#define I2C_DEV "/dev/i2c-1"
#define ACC_I2C_ADDR 0x18

#define CTRL_REG1 0x20
#define POWER_ON  0x27  
#define POWER_IDLE 0x00

#define OUT_X_L 0x28
#define OUT_Y_L 0x2A
#define OUT_Z_L 0x2C


int i2c_fd;

void i2c_init() {
    i2c_fd = open(I2C_DEV, O_RDWR);
    ioctl(i2c_fd, I2C_SLAVE, ACC_I2C_ADDR);
}

void acc_power_on(void) {
    char buffer[2] = {CTRL_REG1, POWER_ON};
    write(i2c_fd, buffer, 2);
}

void acc_power_idle(void) {
    char buffer[2] = {CTRL_REG1, POWER_IDLE};
    write(i2c_fd, buffer, 2);
}

void acc_read_register(char reg, char *val) {
    write(i2c_fd, &reg, 1);
    read(i2c_fd, val, 1);
}

void acc_write_register(char reg, char val) {
    char buffer[2] = {reg, val};
    write(i2c_fd, buffer, 2);
}

float acc_read_acceleration(char axis) {
    char low, high;
    float return_value;

    char reg_low = (axis == 'x') ? OUT_X_L : (axis == 'y') ? OUT_Y_L : OUT_Z_L;
    acc_read_register(reg_low, &low);
    acc_read_register(reg_low + 1, &high);

    if ((int)high > 127){
        return_value = ((float)high * pow(2,8) + (float)low)- pow(2,16);
    }
    else {
        return_value = (float)high * pow(2,8) + (float)low;
    }
    return return_value * 12.0f * 9.81 / pow(2,16);
}

int main(int argc, char*argv[]) {
    i2c_init();
    acc_power_on();
    sleep(1);  

    float acc_x = 0, acc_y = 0, acc_z = 0;
    float alpha = 0.2;  

    while (1) {
        acc_x = acc_read_acceleration('x');
        acc_y = acc_read_acceleration('y');
        acc_z = acc_read_acceleration('z');

        printf("Accélération X: %.3f | Y: %.3f | Z: %.3f\n", acc_x, acc_y, acc_z);

        sleep(1);  
    }

    acc_power_idle();
    close(i2c_fd);
    return 0;
}

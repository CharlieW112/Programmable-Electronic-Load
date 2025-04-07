#include "mbed.h"

BufferedSerial pc(USBTX, USBRX, 9600);
AnalogIn currentSense(PA_0);   // Replace with your ADC pin
AnalogOut vccsOut(PA_5);       // DAC_OUT1 on STM32F429ZI

const int bufferSize = 100;
float buffer[bufferSize];
int bufferIndex = 0;

float mA_to_voltage(int mA) {
    // 0–4000 mA maps to 0–3.3 V
    return (mA / 4000.0f) * 3.3f;
}

void write_dac(int mA) {
    float voltage = mA_to_voltage(mA);
    float norm = voltage / 3.3f;  // AnalogOut expects 0.0–1.0
    if (norm > 1.0f) norm = 1.0f;
    if (norm < 0.0f) norm = 0.0f;
    vccsOut.write(norm);
}

int read_serial_int() {
    char buf[16] = {0};
    int idx = 0;
    while (pc.readable()) {
        char c;
        if (pc.read(&c, 1)) {
            if (c == '\n' || c == '\r') break;
            if (idx < 15) buf[idx++] = c;
        }
    }
    return atoi(buf);
}

int main() {
    pc.set_blocking(false);
    write_dac(0);  // Start at 0 mA

    Timer t;
    t.start();

    while (true) {
        // Read ADC and update rolling buffer
        float val = currentSense.read(); // 0.0–1.0
        buffer[bufferIndex] = val;
        bufferIndex = (bufferIndex + 1) % bufferSize;

        // Average every ~100ms
        if (std::chrono::duration_cast<std::chrono::milliseconds>(t.elapsed_time()).count() >= 100) {
            t.reset();
            float sum = 0;
            for (int i = 0; i < bufferSize; i++) sum += buffer[i];
            float avg = sum / bufferSize;

            char out[64];
            int len = sprintf(out, "%.4f\t%d\n", avg, (int)(vccsOut.read() * 4000));
            pc.write(out, len);
        }

        // Handle user input
        if (pc.readable()) {
            int mA = read_serial_int();
            if (mA >= 0 && mA <= 4000) {
                write_dac(mA);
            } else {
                const char* msg = "Enter a value between 0 and 4000 mA.\n";
                pc.write(msg, strlen(msg));
            }
        }

        ThisThread::sleep_for(10ms);
    }
}

int get_blink_delay_ms(int temperature) {
    if (temperature > 30) {
        return 100;
    } else {
        return 500;
    }
}

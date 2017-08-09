#ifndef CONFIG_H
#define CONFIG_H

#define LOG_PARSER_JSON "log_config.json"
#define AF_DEBUGGER_JSON "af_config.json"

#define SHOW_MORE_LINES_THRESHOLD 200
#define MIN(x, y) (x) > (y) ? (y) : (x)
#define MAX(x, y) (x) > (y) ? (x) : (y)

typedef enum {
    ANDROID_UNKNOWN,
    ANDROID_RUN,
    ANDROID_CLEAR,
    ANDROID_STOP,
    ANDROID_RESUME,
    ANDROID_PAUSE,
} ANDROID_ONLINE_CMD;


#endif // CONFIG_H

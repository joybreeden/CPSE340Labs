#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_SAMPLE_COUNT 10U
#define MAX_SAMPLE_COUNT 100U
#define WARNING_LOW_mC 18000
#define WARNING_HIGH_mC 30000
#define FAILURE_CODE_mC (-999000)
typedef enum {
    MODE_NORMAL,
    MODE_WARNING,
    MODE_FAILURE
} sensor_mode_t;

typedef enum {
    STATUS_OK,
    STATUS_WARNING,
    STATUS_FAILURE
} sensor_status_t;
static double to_celsius(int32_t temperature_mC)
{
    return (double)temperature_mC / 1000.0;
}
static int32_t base_reading(size_t index)
{
    static const int32_t offsets_mC[] = {
        0, 750, -500, 1250, -1000, 500, -250, 1000
    };
    const size_t length = sizeof offsets_mC / sizeof offsets_mC[0];
    return 24000 + offsets_mC[index % length];
}
static int32_t generate_reading(sensor_mode_t mode, size_t index)
{
    const int32_t base = base_reading(index);
    switch (mode)
    {
    case MODE_FAILURE:
        if(index+1 % 5 == 0){
            return FAILURE_CODE_mC;
        }
        else{
            return base;
        }
        break;
    case MODE_WARNING:
        if(index+1 % 4 == 0){
            return 31500;
        }
        else{
            return base;
        }
    default:
        return base;
        break;
    }
    /*  normal mode returns base unchanged. */
    
    /*  warning mode returns 31500 every fourth sample
       (indices 3, 7, 11, ...); otherwise return base. */
    
    /*  failure mode returns FAILURE_CODE_mC every fifth sample
       (indices 4, 9, 14, ...); otherwise return base. */
   
}

static sensor_status_t classify_reading(int32_t temperature_mC)
{  
    if(temperature_mC == FAILURE_CODE_mC){
        return STATUS_FAILURE;
    }
    if(temperature_mC > WARNING_HIGH_mC || temperature_mC < WARNING_LOW_mC){
        return STATUS_WARNING;
    }
    return STATUS_OK;
    /* TODO: test the failure sentinel first. */
    /* TODO: classify valid values using inclusive OK boundaries. */
}


static const char *status_text(sensor_status_t status)
{
    switch (status) {
    case STATUS_OK:      return "OK";
    case STATUS_WARNING: return "WARNING";
    case STATUS_FAILURE: return "FAILURE";
    }
    return "UNKNOWN";
}
int main(){
    int count = 10;
    int ok = 0;
    int warning = 0;
    int failure = 0;
    float ave = 0;
    float min, max;
    float temp;
    for(int i = 0; i < count; i++){
        printf("sample = %d temperature = %fC Status =%s\n",i+1,temp = to_celsius(base_reading(i)), status_text(classify_reading(base_reading(i))));
        switch (classify_reading(base_reading(i)))
        {
        case STATUS_FAILURE:
            failure++;
            break;
        case STATUS_WARNING:
            warning++;
            break;
        default:
            ok++;
            break;
        }
        ave += temp;
        if(i == 0){
            min = temp;
            max = temp;
        }else{
            if(temp < min){
                min = temp;
            }
            if(temp > max){
                max = temp;
            }
        }
    }
    ave = ave / count;

  printf("Summary: samples = %d, valid=%d, ok=%d, warnig=%d, failure=%d\n min = %fC, max = %fC, average = %fC", count,count - failure, ok,warning,failure,min,max,ave);

  }
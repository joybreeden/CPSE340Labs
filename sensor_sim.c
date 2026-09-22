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


static int parse_mode(const char *text,sensor_mode_t *mode)
{
    char normal[] = "normal";
    char warning[] = "warning";
    char failure[] = "failure";
   // printf("warning: %s\n", warning);
  //  printf("text: %s\n",text);
   // printf("compare: %d\n",strcmp(warning,text));
    if (strcmp(normal,text)==0){
        *mode = MODE_NORMAL;
        return 0;
    }
     if (strcmp(warning,text)==0){
        *mode = MODE_WARNING;
        return 0;
    }
     if (strcmp(failure,text)==0){
        *mode = MODE_FAILURE;
        return 0;
    }
    return -1;
    /* Returns 0 on success and -1 on invalid arguments or mode text. */
    /* Use strcmp() to accept exactly: normal, warning, failure. */
}



static int parse_count(const char *text, size_t *count)
{
    char *end = NULL;
    unsigned long value;

    if (text == NULL || count == NULL || text[0] == '\0') return -1;
    errno = 0;
    value = strtoul(text, &end, 10);
    //printf("%d\n",value);
    if (errno != 0 || end == text || *end != '\0'){
             return -1;}
    if(value < 1 || value > MAX_SAMPLE_COUNT){
        return -1;
    }
    *count = (size_t)value;
    return 0;

}



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
    //printf("Mode: %d, warning: %d", mode, MODE_WARNING);
    switch (mode)
    {
    case MODE_FAILURE:
        if((index+1) % 5 == 0){
            return FAILURE_CODE_mC;
        }
        else{
            return base;
        }
        break;
    case MODE_WARNING:
        //printf("good");
        if((index+1) % 4 == 0){
            //printf("good");
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



int main(int argc, char *argv[]){
    size_t ok = 0;
    sensor_mode_t mode;
    size_t count = 10;
    size_t warning = 0;
    size_t failure = 0;
    double ave = 0;
    double min, max;
    double temp;
    int32_t tempu;
    if(argc > 3 || argv[1]== NULL){
        printf("failure args");
        return 0;
    }
    if(parse_mode(argv[1],&mode) == -1){
        printf("Failure Mode");
        return 0;
    }if(argv[2] == NULL){
        count = 10;
    }else if(parse_count(argv[2],&count) == -1){
        printf("failure count");
        return 0;
    
    }
    for(size_t i = 0; i < count; i++){
         printf("sample = %lld ",i+1);
        tempu = generate_reading(mode,i);
        if(tempu != FAILURE_CODE_mC){
            temp = to_celsius(tempu);
            printf("temperature = %fC",temp);
         }else{
             printf("temperature = Null");

         }

            
         printf( " Status =%s\n", status_text(classify_reading(generate_reading(mode,i))));
        switch (classify_reading(base_reading(i)))
        {
        case STATUS_FAILURE:
            failure++;
            continue;
            break;
        case STATUS_WARNING:
            warning++;
            break;
        default:
            ok++;
            break;
        }
        if(temp != FAILURE_CODE_mC){
            ave += temp;
        }
        if(i == 0){
            if(temp != FAILURE_CODE_mC){
             min = temp;
                max = temp;
            }
        }else{
            if(temp < min){
                if(temp != FAILURE_CODE_mC)
                 min = temp;
            }
            if(temp > max){
                if(temp != FAILURE_CODE_mC)
                    max = temp;
            }
        }
    }
    ave =  ave / (double)count ;

  printf("Summary: samples = %lld, valid=%lld, ok=%lld, warning=%lld, failure=%lld\n min = %fC, max = %fC, average = %fC", count,count - failure, ok,warning,failure,min,max,ave);

  }
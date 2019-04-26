#include "temperature.h"
#include <OneWire.h>
#include <DallasTemperature.h>

///////////////////////////// private

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);


///////////////////////////// public

void setupTemperature() {
    sensors.begin();
    sensors.setResolution(RESOLUTION);
}

float getTemperature() {
    #ifdef USE_TEMP_SENSOR_ADDRESS
        sensors.requestTemperaturesByAddress(TEMP_SENSOR_ADDRESS);
        return sensors.getTempC(TEMP_SENSOR_ADDRESS);
    #else
        sensors.requestTemperaturesByIndex(0);
        return sensors.getTempCByIndex(0);
    #endif
}

#include "OTA.h"

#include "DevelDeckAPI.h"

void progressCallBack(size_t currSize, size_t totalSize) {
      Serial.printf("OTA:  Update process at %d of %d bytes...\n", currSize, totalSize);

      ddeck.game_downloading_screen( (float) currSize / totalSize * 100.0f );
}

bool OTA_update(File &firmware){
    if(!firmware || firmware.isDirectory())
        return 0;

    Update.onProgress(progressCallBack);

    if(!Update.begin(firmware.size(), U_FLASH))
        return 0;
    Update.writeStream(firmware);

    if (!Update.end()){
        Serial.println("OTA:  Update error!");
        Serial.println(Update.getError());
        return 0;
    }
    Serial.println("OTA:  Update finished!");

    return 1;
}

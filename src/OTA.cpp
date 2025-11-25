#include "OTA.h"

#include "DevelDeckAPI.h"

void progressCallBack(size_t currSize, size_t totalSize) {
      Serial.printf("CALLBACK:  Update process at %d of %d bytes...\n", currSize, totalSize);

      gamepad.game_downloading_screen( (float) currSize / totalSize * 100.0 );
}

bool OTA_update(File &firmware){
    if(!firmware || firmware.isDirectory())
        return 0;

    Update.onProgress(progressCallBack);

    Update.begin(firmware.size(), U_FLASH);
    Update.writeStream(firmware);

    if (Update.end())
          Serial.println("Update finished!");
    else{
        Serial.println("Update error!");
        Serial.println(Update.getError());
        return 0;
    }

    return 1;
}

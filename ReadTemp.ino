void ReadTemp(){
  if(numberOfDevices == 0){
      Temp0 = 999.0;
      return;
  }
  float t = 0;
  Temp.requestTemperatures(); // Send the command to get temperatures  
    if(Temp.getAddress(tempDeviceAddress, 0)){
      
    // Output the device ID
       t = Temp.getTempC(tempDeviceAddress);
       if(t > 65000)t = 65000;
          Serial.print("Temperature for device: ");
          printAddress(tempDeviceAddress);
          Serial.print(" Temp= ");Serial.println(t,1);
       }

  Temp0 = t;
}

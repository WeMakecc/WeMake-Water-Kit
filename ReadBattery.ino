void ReadBattery(){
  int16_t adc1 = 0;
  adc1 = ads1115_0.readADC_SingleEnded(pinBAT);
  vBat = Vconv*adc1*2.0/1000.0;
  Serial.print(" vBat ");Serial.println(vBat);
}

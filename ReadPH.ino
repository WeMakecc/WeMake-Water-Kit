void ReadPH(){
  float A1 = 0;  //  coefficient first order
  float A0 = 0;  //  coefficient 0 order
  int16_t   adc0  = 0;
  float     Volt0 = 0;
  uint16_t  nsample = 10;
  float     sum = 0.;
// regression for Probe #1  // Pampurini 
  if(pHprobe == 1){
    A1 = -0.0175;
    A0 = 38.841;
  }

// regression for Probe #2
  if(pHprobe == 2){
    A1 = -0.0183;
    A0 = 39.97;
  }

// regression for Probe #3
  if(pHprobe == 3){
    A1 = -0.0177;
    A0 = 38.936;
  }

// regression for Probe #4
  if(pHprobe == 4){
    A1 = -0.0192;
    A0 = 41.496;
  }
  
// regression for Probe #5
  if(pHprobe == 5){
    A1 = -0.0176;
    A0 = 38.577;
  }
  
// regression for Probe #6  // Pampurini scheda ampli Logo-Rnaenaor V2.0
  if(pHprobe == 6){
    A1 = -0.0059;
    A0 = 21.823;
  }  
  for(int i = 0; i < nsample; i++){
     adc0 = ads1115_0.readADC_SingleEnded(OUT);
     delay(10);
     Volt0 = float(adc0)*Vconv;
     Serial.print(" pH Raw: ");Serial.print(adc0);Serial.print(" >> ");
     Serial.print(Volt0,0);Serial.println(" mV ");
     sum = sum + Volt0;
  }
  sum = sum/nsample;
  pHvolt = sum;
  Serial.print(" Average ");Serial.print(pHvolt,0);Serial.println(" mV ");
//  Convert mV in pH units
  PH = A1*pHvolt + A0;
  if(PH < 2 || PH > 11)PH = 99.9;
}

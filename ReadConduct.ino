void ReadConduct(int16_t nAverage, int16_t nmeno){
  
/*   Conduttimetro
Versione 28/11/2020 
Paolo Bonelli
nAverage: number of samples
nmeno   : number of the first samples to be rejected
R1      : fixed resistor in ohm
This function computes specific conductivity in uS/cm from measured 
resistance of the water and its temperature.
The probe is made by two stainless steel bolt  M4 x 8mm mounted on a PVC disc 
at the distance of about 20 mm.
Temperature correction is obtained from:
RScor(20°) = RS(T)/fact(T)  where RScor is the water resistance at 20°C and RS(T) 
is the resistance at temperature T.
fact(T) = 0.0005*T^2 -0.0414*T + 1.63  relazione quadratica con errori intorno a 1%

 *****************************************************************************************   
 Wiring for the Conductivity Probe

           yellow             green               Red
           P1 -------O  O------PA---------R1-------P2   
            |.........Vo2......|.........Vo1........|  
*/
//  calibration coefficient
float    A0 = 0;  // coefficient 0 order  
float    A1 = 0;  // coefficient first order  
float    A2 = 0;  // coefficient second order
float    R1 = 0;  // Resistance of the voltage divider

float    VV =      12.0;
int16_t  nmilli =  1000;    // delay in millis after switch on P1 or P2 and before reading PA
float    Reduct =  0.3125;   // divider factor for PA
float    Avg1 =    0;
float    Avg2 =    0;
int16_t  i =       0;
float    RS =      0.;
float    RS1 =     0.;
float    RS2 =     0.;
float    Vo1 =     0.;
float    Vo2 =     0.;
float    fact =    0.;
int16_t  adc1 =    0;

//  init EC probe
switch(ECprobe){
  case 1:    // Pampurini 
    R1 = 466.0;
    A0=  4.0291;
    A1=  0.5468;
    A2=  0.0014;
    break;
  case 2:
    R1 = 472.0;
    A0=  -65.711;
    A1=  0.8205;
    A2=  0.0009;
    break;
  case 3:
    R1 = 467.0;
    A0=  -18,689;
    A1=  0.397;
    A2=  0.0015;
    break;
  case 4:
    R1 = 466.0;
    A0=  -34.325;
    A1=  0.4356;
    A2=  0.0012;
    break;
  case 5:
    R1 = 470.0;
    A0=  -100.74;
    A1=  1.2981;
    A2=  0.0004;
    break;
  case 6:   // Pampurini da calibrare
    R1 = 467.0;  //
    A0=  -75.662;
    A1=  1.106;
    A2=  0.0003;
    break;

  default:
    lcd.clear();
    lcd.println("Wrong ECprobe");
    break;
}

//   Samples and averages 
       Avg1 = 0;
       Avg2 = 0;
       for(i = 0; i < nAverage; i++){
           // sensor power supply for measuring Vo1: P1 on and P2 off
           digitalWrite(ContPin1,HIGH);
           digitalWrite(ContPin2,LOW);
           delay(nmilli);
           adc1 = ads1115_0.readADC_SingleEnded(PA);
           float pippo = Vconv*adc1/1000.0;
           Serial.print(pippo,3);Serial.print(" "); 
           if(i >= nmeno){
             Avg1 =  Avg1 + pippo;
           }
           
          // sensor power supply for measuring Vo2: P1 off and P2 on
           digitalWrite(ContPin1,LOW);
           digitalWrite(ContPin2,HIGH);
           delay(nmilli);
           adc1 = ads1115_0.readADC_SingleEnded(PA);
           pippo = Vconv*adc1/1000.0;
           Serial.print(pippo,3);         
           if(i >= nmeno){
              Avg2 =  Avg2 + pippo;
           }
           Serial.println();
       }  
       digitalWrite(ContPin1,LOW);
       digitalWrite(ContPin2,LOW);
       delay(nmilli);
       adc1 = ads1115_0.readADC_SingleEnded(PA);
       float Vg = Vconv*adc1/1000.0;
       Vo1 = Avg1/(Reduct*(nAverage-nmeno));      
       Vo2 = Avg2/(Reduct*(nAverage-nmeno));   
       Serial.println(" -----------------Measures--------------- ");
       Serial.print("  Vo1 = ");
       Serial.print(Vo1,3);
       Serial.print(" Vo2 = ");
       Serial.println(Vo2,3);
       Serial.print(" Galvanic Potential ");Serial.println(Vg,3);    

//  correction for Galvanic potential
       Vo1 = Vo1 - Vg;
//  !!!Absolute function (abs) doesn't work with float numbers!!!
       if(Vo1 < 0)Vo1 = -Vo1;
       Vo2 = Vo2 - Vg;
       if(Vo2 < 0)Vo2 = -Vo2;
       
//  compute RS1
        if(Vo1 < 0.2){
            RS1 = 99999.;
         }else{
            RS1 = R1*(VV-Vo1)/Vo1;
         } 
         if((VV-Vo2) < 0.2){
            RS2 = 99999.;
          }else{
            RS2 = R1*Vo2/(VV-Vo2);
          }
         Serial.print(" RS1 ");Serial.println(RS1);
         Serial.print(" RS2 ");Serial.println(RS2);
        
//  Averages the two values RS1 and RS2
          if(RS1 < 60000. && RS2 < 60000.){
             RS = (RS1 + RS2)/2; 
             if(RS < 50.){
                CS = 0;
                CSspec = 0;
             }else{
//   Compute the temperature correction factor CS(20 degree) = fact(T)*CS(T)
                if(Temp0 > 900.0){
                   fact = 1.0;
                }else{
//                 second order regression with errors of about 1%
                   fact = 0.0005*pow(Temp0,2) - 0.0414*Temp0 + 1.63;
                }

// Computes conductivity in microS from RS and corrects it for temperature
                CS = fact*1000000/RS;  //  Conductivity a 20°
//  Computes specific conductivity
                CSspec = A0 + A1*CS + A2*CS*CS;
                if(CSspec < 0)CSspec = 0;
            }
        }else{
          RS =     99999.;
          CS =     99999.;
          CSspec = 99999.; 
          RS2 =    99999.;
          RS1 =    99999.; 
        }
         Serial.print("  RS1 = ");
         Serial.print(RS1,0);
         Serial.print("  RS2 = ");
         Serial.print(RS2,0);
         Serial.print("  RS = ");
         Serial.println(RS,0);
         Serial.println(" -------------------------------------- ");
}

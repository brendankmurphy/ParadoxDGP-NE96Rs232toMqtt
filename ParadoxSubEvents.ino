String getSubEvent(byte event, byte sub_event)
{
 
    switch (event)
    {
      case 00:
        return "";
      break;
      
      case 01:
        return "";
      break;
  
      case 02:
        return "";
      break;
      
    case 03:
        return "";
      break;
      
    case 04:
         switch (sub_event)
        {
          case 0: 
           return F("TLM Trouble");
          break;

          case 1: 
           return F("Smoke Detector Reset");
          break;

          case 2:
           return F("Arm with no entry delay");
          break;

          case 3: 
           return F("Arm in Stay mode");
          break;

          case 4: 
           return F("Arm in Away mode");
          break;

          case 5: 
           return F("Full Arm when in Stay Mode");
          break;

          case 6: 
           return F("Voice Module Access");
          break;

          case 7: 
           return F("Remote Control Access");
          break;

          case 8: 
           return F("PC Fail to Communicate");
          break;

          case 9: 
           return F("Midnight");
          break;

          case 10: 
           return F("NEware User Login");
          break;

          case 11: 
           return F("NEware User Logout");
          break;

          case 12: 
           return F("User initiated callup");
          break;

          case 13: 
           return F("Force Answer");
          break;

          case 14: 
           return F("Force Hangup");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 05:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 07:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 8:
      return "";
    break;

    case 9:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 10:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 12:
         switch (sub_event)
        {
          case 0: 
           return F("Auto Arming");
          break;

          case 1: 
           return F("Arming with WinLoad");
          break;

          case 2:
           return F("Late to Close");
          break;

          case 3: 
           return F("No Movement Arming");
          break;

          case 4: 
           return F("Partial Arming");
          break;

          case 5: 
           return F("One-touch Arming");
          break;

          case 6: 
           return F("Future Use");
          break;

          case 7: 
           return F("Future Use");
          break;

          case 8: 
           return F("Voice Module Arming");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 13:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 14:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 16:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 17:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 19:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 20:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 22:
         switch (sub_event)
        {
          case 0: 
           return F("Auto Arm Cancelled");
          break;

          case 1: 
           return F("One-touch Stay/Instant Disarm");
          break;

          case 2:
           return F("Disarming with WinLoad");
          break;

          case 3: 
           return F("Disarming with WinLoad after alarm");
          break;

          case 4: 
           return F("WinLoad Cancelled Alarm");
          break;

          case 5: 
           return F("Future Use");
          break;

          case 6: 
           return F("Future Use");
          break;

          case 7: 
           return F("Future Use");
          break;

          case 8: 
           return F("Voice Module Arming");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 23:
            return "";
          break;

    case 24:
            return "";
          break;

    case 26:
        return "";
      break;

    case 28:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 29:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 30:
        switch (sub_event)
        {
          case 0: 
           return F("Emergency Panic");
          break;

          case 1: 
           return F("Medical Panic");
          break;

          case 2: 
           return F("Fire Panic");
          break;


          case 3: 
           return F("Recent Closing");
          break;

          case 4: 
           return F("Police Code");
          break;

          case 5: 
           return F("Global Shutdown");
          break;

          default:
           return "";
           break;
        }
      break;

    case 31:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 32:
        return "";
      break;

    case 33:
        return "";
      break;
      
    case 34:
        return "";
      break;

    case 35:
        return "";
      break;

    case 36:
         switch (sub_event)
        {
          case 0: 
           return F("TLM Trouble");
          break;

          case 1: 
           return F("AC Failure");
          break;

          case 2: 
           return F("Battery Failure");
          break;

          case 3: 
           return F("Auxilliary Current Limit");
          break;

          case 4: 
           return F("Bell Current Limit");
          break;

          case 5: 
           return F("Bell Absent");
          break;

          case 6: 
           return F("Clock Trouble");
          break;

          case 7: 
           return F("Global Fire Loop");
          break;

          default:
          return "";
          break;
        }
      break;

    case 37:
         switch (sub_event)
        {
          case 0: 
           return F("TLM Trouble");
          break;

          case 1: 
           return F("AC Failure");
          break;

          case 2: 
           return F("Battery Failure");
          break;

          case 3: 
           return F("Auxilliary Current Limit");
          break;

          case 4: 
           return F("Bell Current Limit");
          break;

          case 5: 
           return F("Bell Absent");
          break;

          case 6: 
           return F("Clock Trouble");
          break;

          case 7: 
           return F("Global Fire Loop");
          break;

          default:
          return "";
          break;
        }
      break;

    case 38:
         switch (sub_event)
        {
          case 0: 
           return F("ComBus Fault");
          break;

          case 1: 
           return F("Module Tamper");
          break;

          case 2: 
           return F("ROM/RAM Error");
          break;

          case 3: 
           return F("TLM Trouble");
          break;

          case 4: 
           return F("Fail to Communicate");
          break;

          case 5: 
           return F("Printer Fault");
          break;

          case 6: 
           return F("AC Failure");
          break;

          case 7: 
           return F("Battery Failure");
          break;
          
          case 8: 
           return F("Auxilliary Failure");
          break;
          
          default:
          return "";
          break;
        }
      break;

    case 39:
         switch (sub_event)
        {
          case 0: 
           return F("ComBus Fault");
          break;

          case 1: 
           return F("Module Tamper");
          break;

          case 2: 
           return F("ROM/RAM Error");
          break;

          case 3: 
           return F("TLM Trouble");
          break;

          case 4: 
           return F("Fail to Communicate");
          break;

          case 5: 
           return F("Printer Fault");
          break;

          case 6: 
           return F("AC Failure");
          break;

          case 7: 
           return F("Battery Failure");
          break;
          
          case 8: 
           return F("Auxilliary Failure");
          break;
          
          default:
          return "";
          break;
        }
      break;

    case 40:
        return "";
      break;

    case 41:
        return "";
      break;

    case 42:
        return "";
      break;

    case 43:
        return "";
      break;

    case 44:
        return "";
      break;
      
    case 45:
         switch (sub_event)
        {
          case 0: 
           return F("Power up after total power down");
          break;

          case 1: 
           return F("Software reset (Watchdog)");
          break;

          case 2: 
           return F("Test Report");
          break;

          case 3: 
           return F("Future Use");
          break;

          case 4: 
           return F("WinLoad In (connected)");
          break;

          case 5: 
           return F("WinLoad Out (disconnected)");
          break;

          case 6: 
           return F("Installer in Programming");
          break;

          case 7: 
           return F("Installer out of Programming");
          break;

           default:
          return "";
          break;

        }
      break;

    case 46:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 47:
      switch (sub_event)
        {
          case 02: 
           return F("User Codes");
          break;

          default: 
           return "";
          break;
        }
      break;

    case 55:
        return "";
      break;

    case 56:
        return "";
      break;

    case 57:
        return "";
      break;

    case 64:
         switch (sub_event)
        {
          case 0: 
           return F("Armed");
          break;

          case 1: 
           return F("Force Armed");
          break;

          case 2: 
           return F("Stay Armed");
          break;

          case 3: 
           return F("Instant Armed");
          break;

          case 4: 
           return F("Strobe Alarm");
          break;

          case 5: 
           return F("Silent Alarm");
          break;

          case 6: 
           return F("Audible Alarm");
          break;

          case 7: 
           return F("Fire Alarm");
          break;

           default:
          return "";
          break;

        }
      break;

    case 65:
         switch (sub_event)
        {
          case 0: 
           return F("Ready");
          break;

          case 1: 
           return F("Exit Delay");
          break;

          case 2: 
           return F("Entry Delay");
          break;

          case 3: 
           return F("System in Trouble");
          break;

          case 4: 
           return F("Alarm in Memory");
          break;

          case 5: 
           return F("Zones Bypassed");
          break;

          case 6: 
           return F("Bypass, Master, Installer Programming");
          break;

          case 7: 
           return F("Keypad Lockout");
          break;

           default:
          return "";
          break;

        }
      break;

    case 66:
         switch (sub_event)
        {
          case 0: 
           return F("Intellizone Delay Engaged");
          break;

          case 1: 
           return F("Fire Delay Engaged");
          break;

          case 2: 
           return F("Auto Arm");
          break;

          case 3: 
           return F("Arming with Voice Module");
          break;

          case 4: 
           return F("Tamper");
          break;

          case 5: 
           return F("Zone Low Battery");
          break;

          case 6: 
           return F("Fire Loop Trouble");
          break;

          case 7: 
           return F("Zone Supervision Trouble");
          break;

           default:
          return "";
          break;

        }
      break;
    }
      
}

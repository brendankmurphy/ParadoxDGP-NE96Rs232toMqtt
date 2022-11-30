
String getEvent(byte event)
{
  switch (event)
    {
      case 00:
        return F("Zone OK");
      break;
      
      case 01:
        return F("Zone Open");
      break;
  
      case 02:
        return F("Zone is tampered");
      break;

    case 03:
        return F("Zone in fire loop status");
      break;
      
    case 04:
        return F("Non-reportable event");
      break;

    case 05:
        return F("User code entered on keypad");
      break;

    case 07:
        return F("Bypass programming access");
      break;

    case 8:
        return F("TX Delay Zone Alarm");
      break;

    case 9:
        return F("Arming with Master");
      break;

    case 10:
        return F("Arming with User Code");
      break;

    case 12:
        return F("Special Arming");
      break;

    case 13:
        return F("Disarm with Master");
      break;

    case 14:
        return F("Disarm with User Code");
      break;

    case 16:
        return F("Disarm after alarm with Master");
      break;

    case 17:
        return F("Disarm after alarm with User Code");
       break;
    
    case 19:
            return F("Alarm cancelled with Master");
          break;

    case 20:
        return F("Alarm cancelled with User Code");
      break;

    case 22:
        return F("Special Disarm Events");
      break;

    case 23:
        return F("Zone bypassed");
      break;

    case 24:
        return F("Zone in Alarm");
      break;

    case 26:
        return F("Zone Alarm Restore");
      break;

    case 28:
        return F("Early to Disarm by User");
      break;

    case 29:
        return F("Late to Disarm by User");
      break;

    case 30:
        return F("Special Alarm");
      break;

    case 31:
        return F("Duress Alarm by User");
      break;

    case 32:
        return F("Zone shutdown");
      break;

    case 33:
        return F("Zone tamper");
      break;

    case 34:
        return F("Zone tamper restore");
      break;

    case 35:
        return F("Special Tamper");
      break;

    case 36:
        return F("Trouble Event");
      break;

    case 37:
        return F("Trouble Restore");
      break;

    case 38:
        return F("Module Trouble");
      break;

    case 39:
        return F("Module Trouble Restore");
      break;

    case 41:
        return F("Low Battery on Zone");
      break;

    case 42:
        return F("Zone Supervision Trouble");
      break;

    case 43:
        return F("Low Battery on Zone Restored");
      break;

    case 44:
        return F("Zone Supervision Trouble Restored");
      break;

    case 45:
        return F("Special Events");
      break;

    case 46:
        return F("Early to Arm by User");
      break;

    case 47:
        return F("Late to Arm by User");
      break;

    case 55:
        return F("Intellizone Triggered");
      break;

    case 56:
        return F("Zone excluded on Force Arming");
      break;

    case 57:
        return F("Zone went back to Arm Status");
      break;

    case 64:
        return F("Status 1");
      break;

    case 65:
        return F("Status 2");
      break;
      
    case 66:
        return F("Status 3");
      break;

    default:
      return "";
      break;

    }
}

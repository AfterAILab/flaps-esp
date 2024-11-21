#include "Timezone.h"
#include <ezTime.h>
#include "nvsUtils.h"
#include "env.h"

/**
 * @purpose Global timezone object encapsulates user's timezone state variable
 * @caller getDateString(), getClockString()
 */
Timezone timezone;

/**
 * @purpose Update the global timezone object with user's timezone string
 * @caller POST /misc handler
 */
void applyUserTimezone()
{
  String timezoneString = getNvsString("timezone", "Asia/Tokyo");
  timezone.setLocation(timezoneString);
}

/**
 * @purpose Get the current date string in the user's timezone
 * @caller showDate()
 */
String getDateString()
{
  return timezone.dateTime(DATE_FORMAT);
}

/**
 * @purpose Get the current clock string in the user's timezone
 * @caller showClock()
 */
String getClockString()
{
  return timezone.dateTime(CLOCK_FORMAT);
}
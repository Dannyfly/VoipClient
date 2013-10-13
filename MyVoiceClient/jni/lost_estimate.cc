#include <cstdlib>

#include <android/log.h>
#define THIS_FILE " android_test.cc "
#define MYLOG(...) __android_log_print(ANDROID_LOG_DEBUG, THIS_FILE, __VA_ARGS__)


int isPacketLoss(int lossPercentage)
{
	MYLOG("Info: isPacketLoss() lossPercentage=%d", lossPercentage);
	if (rand() % 100 <= lossPercentage)
	{
		return 1;	// Loss
	}

	return 0;	// Not Loss
}


namespace webrtc
{
	namespace test
	{
		int lost_rate = 5;
		int continue_lost_rate = 2;

		int nettrans_estimate()
		{
			return 1;
			static int flag = 0;
			static int lost_level = 0;
			if (flag == 0)
			{
				if (rand() % 100 < lost_rate)
				{
					lost_level = rand()%continue_lost_rate + 1;
					return 0;
				}
			}
			else if (flag < lost_level-1)
			{
				flag++;
				return 0;
			}
			else if (flag == lost_level-1)
			{
				flag = 0;
			}
			return 1;
		}
	}
}


namespace webrtc
	{
		int lost_rate = 5;
		int continue_lost_rate = 2;

		int nettrans_estimate()
		{
			return 1;
			static int flag = 0;
			static int lost_level = 0;
			if (flag == 0)
			{
				if (rand() % 100 < lost_rate)
				{
					lost_level = rand()%continue_lost_rate + 1;
					return 0;
				}
			}
			else if (flag < lost_level-1)
			{
				flag++;
				return 0;
			}
			else if (flag == lost_level-1)
			{
				flag = 0;
			}
			return 1;
		}
	}
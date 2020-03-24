#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "porting.h"
#include "iot_stream.h"
#include "protocol.h"

static int __Process(stream_param_t *params , int params_num)
{
    int i = 0;
    stream_param_t * pone = params;

    for(i = 0; pone && i < params_num; i++ , pone++){
        switch(pone->pid){
        	case HT2ME_SLIGHT_SWITCH:
                DEBUG_PRINTF("控制开关:%d\n",pone->value);
                break;
        	case HT2ME_SLIGHT_COLOR_R:
                DEBUG_PRINTF("控制因素R:%d\n",pone->value);
                break;
        	case HT2ME_SLIGHT_COLOR_G:
                DEBUG_PRINTF("控制因素G:%d\n",pone->value);
                break;
        	case HT2ME_SLIGHT_COLOR_B:
                DEBUG_PRINTF("控制因素B:%d\n",pone->value);
                break;
        	case HT2ME_SLIGHT_COLOR_C:
                DEBUG_PRINTF("控制因素C:%d\n",pone->value);
                break;
        	case HT2ME_SLIGHT_COLOR_H:
                DEBUG_PRINTF("控制因素H:%d\n",pone->value);
                break;
        	case HT2ME_SLIGHT_MODE:
                DEBUG_PRINTF("控制灯光模式:%d\n",pone->value);
                break;
        	case HT2ME_SLIGHT_BLURRY:
                DEBUG_PRINTF("控制模糊:%d\n",pone->value);
                break;
        	case HT2ME_SLIGHT_DIRECT_COLOR:
                DEBUG_PRINTF("控制目标颜色:%d\n",pone->value);
                break;
        	case HT2ME_SLIGHT_ANTIFOG:
                DEBUG_PRINTF("控制防雾开关:%d\n",pone->value);
                break;

            case HT2ME_REAUEST_ALL_STATUS:
                DEBUG_PRINTF("查询当前状态:%d\n",pone->value);
                break;
            case HT2ME_UPDATE_RUNMODE:
                DEBUG_PRINTF("HT当前为:%d\n",pone->value);
                break;
            case HT2ME_UPDATE_PLAYSTATUS:
                DEBUG_PRINTF("HT当前播放状态:%d\n",pone->value);
                break;
            case HT2ME_UPDATE_WIFISTATUS:
                DEBUG_PRINTF("HT当前WIFI状态:%d\n",pone->value);
                break;
            case HT2ME_UPDATE_BTSTATUS:
                DEBUG_PRINTF("HT当前BT状态:%d\n",pone->value);
                break;
            case HT2ME_UPDATE_PLAYMODE:
                DEBUG_PRINTF("HT当前播放模式:%d\n",pone->value);
                break;
            case HT2ME_UPDATE_VOLUME:
                DEBUG_PRINTF("HT当前音量:%d\n",pone->value);
                break;
            case HT2ME_UPDATE_SYSTEMTIME:
                DEBUG_PRINTF("HT当前时间戳:%d\n",pone->value);
                break;

            default:
                ERROR_PRINTF("not supported pid\n");
                break;
        }
    }

    return 0;
}

int IOTProcess(stream_msg_t ** ppstream , uint8_t *streamb , int streamlen)
{
    stream_msg_t * pstream = NULL;

    int result = HTIOTParseToStream(&pstream,(unsigned char *)streamb , streamlen);
    if(result==0){
        stream_param_t * params = (stream_param_t *)pstream->body;
        int param_num = pstream->bodylen / sizeof(stream_param_t);

        __Process(params , param_num);

    }

	*ppstream = pstream;

    return result;
}

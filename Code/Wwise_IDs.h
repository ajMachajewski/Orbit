/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Audiokinetic Wwise generated include file. Do not edit.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __WWISE_IDS_H__
#define __WWISE_IDS_H__

#include <AK/SoundEngine/Common/AkTypes.h>

namespace AK
{
    namespace EVENTS
    {
        static const AkUniqueID PLAY_DEMOMUSIC = 2121299920U;
        static const AkUniqueID PLAY_DUNE = 2560865840U;
        static const AkUniqueID PLAY_PLAYERDEATH = 910581297U;
        static const AkUniqueID PLAY_RIZUSTAGE = 3429943800U;
        static const AkUniqueID PLAY_TESTCLICK = 83581882U;
        static const AkUniqueID SLOW_MUSIC = 862296290U;
        static const AkUniqueID STOP_DEMOMUSIC = 394383334U;
        static const AkUniqueID STOP_DUNE = 2845582322U;
        static const AkUniqueID STOP_RIZUSTAGE = 98427794U;
    } // namespace EVENTS

    namespace STATES
    {
        namespace DEFAULT
        {
            static const AkUniqueID GROUP = 782826392U;

            namespace STATE
            {
                static const AkUniqueID DEFAULT = 782826392U;
                static const AkUniqueID NONE = 748895195U;
                static const AkUniqueID SLOW = 787604482U;
            } // namespace STATE
        } // namespace DEFAULT

        namespace SLOWING
        {
            static const AkUniqueID GROUP = 628919086U;

            namespace STATE
            {
                static const AkUniqueID NONE = 748895195U;
            } // namespace STATE
        } // namespace SLOWING

    } // namespace STATES

    namespace GAME_PARAMETERS
    {
        static const AkUniqueID MUSICPLAYBACKSPEED = 1440783398U;
    } // namespace GAME_PARAMETERS

    namespace BANKS
    {
        static const AkUniqueID INIT = 1355168291U;
        static const AkUniqueID MAIN = 3161908922U;
    } // namespace BANKS

    namespace BUSSES
    {
        static const AkUniqueID MASTER_AUDIO_BUS = 3803692087U;
    } // namespace BUSSES

    namespace AUDIO_DEVICES
    {
        static const AkUniqueID NO_OUTPUT = 2317455096U;
        static const AkUniqueID SYSTEM = 3859886410U;
    } // namespace AUDIO_DEVICES

}// namespace AK

#endif // __WWISE_IDS_H__

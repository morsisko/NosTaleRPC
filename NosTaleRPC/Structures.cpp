#include "Structures.h"

PlayerIcon * CharacterInfo::GetIcon()
{
	CAN_READ_POINTER(CharacterInfo);
	return m_pPlayerIcon;
}

IconInformation * PlayerIcon::GetInformation()
{
	CAN_READ_POINTER(PlayerIcon);
	return m_pIconInformation;
}

short IconInformation::GetId()
{
	CAN_READ(IconInformation);
	return m_sIconId;
}

bool TimespaceInformation::IsInTimespace()
{
	CAN_READ_BOOL(TimespaceInformation);
	return m_bIsInTimespace;
}

int WaveTimer::GetTimeToEnd()
{
	CAN_READ(WaveTimer);
	return m_iTimeToEnd;
}

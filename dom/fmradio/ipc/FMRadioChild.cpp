/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FMRadioChild.h"
#include "mozilla/dom/ContentChild.h"
#include "mozilla/dom/FMRadioRequestChild.h"

using namespace mozilla::hal;

BEGIN_FMRADIO_NAMESPACE

StaticAutoPtr<FMRadioChild> FMRadioChild::sFMRadioChild;

FMRadioChild::FMRadioChild()
  : mEnabled(false)
  , mFrequency(0)
  , mObserverList(FMRadioEventObserverList())
  , mPi(0)
  , mEcc(0)
{
  MOZ_COUNT_CTOR(FMRadioChild);

  ContentChild::GetSingleton()->SendPFMRadioConstructor(this);

  StatusInfo statusInfo;
  SendGetStatusInfo(&statusInfo);

  mEnabled = statusInfo.enabled();
  mFrequency = statusInfo.frequency();
  mUpperBound = statusInfo.upperBound();
  mLowerBound= statusInfo.lowerBound();
  mChannelWidth = statusInfo.channelWidth();
}

FMRadioChild::~FMRadioChild()
{
  MOZ_COUNT_DTOR(FMRadioChild);
}

bool
FMRadioChild::IsEnabled() const
{
  return mEnabled;
}

double
FMRadioChild::GetFrequency() const
{
  return mFrequency;
}


double
FMRadioChild::GetFrequencyUpperBound() const
{
  return mUpperBound;
}

double
FMRadioChild::GetFrequencyLowerBound() const
{
  return mLowerBound;
}

double
FMRadioChild::GetChannelWidth() const
{
  return mChannelWidth;
}

uint16_t
FMRadioChild::GetPi() const
{
  return mPi;
}

uint8_t
FMRadioChild::GetEcc() const
{
  return mEcc;
}

void
FMRadioChild::GetPs(nsString& aPs) const
{
  aPs = mPs;
}

void
FMRadioChild::GetRt(nsString& aRt) const
{
  aRt = mRt;
}

void
FMRadioChild::Enable(double aFrequency, FMRadioReplyRunnable* aReplyRunnable)
{
  SendRequest(aReplyRunnable, EnableRequestArgs(aFrequency));
}

void
FMRadioChild::Disable(FMRadioReplyRunnable* aReplyRunnable)
{
  SendRequest(aReplyRunnable, DisableRequestArgs());
}

void
FMRadioChild::SetFrequency(double aFrequency,
                           FMRadioReplyRunnable* aReplyRunnable)
{
  SendRequest(aReplyRunnable, SetFrequencyRequestArgs(aFrequency));
}

void
FMRadioChild::Seek(FMRadioSeekDirection aDirection,
                   FMRadioReplyRunnable* aReplyRunnable)
{
  SendRequest(aReplyRunnable, SeekRequestArgs(aDirection));
}

void
FMRadioChild::CancelSeek(FMRadioReplyRunnable* aReplyRunnable)
{
  SendRequest(aReplyRunnable, CancelSeekRequestArgs());
}

inline void
FMRadioChild::NotifyFMRadioEvent(FMRadioEventType aType)
{
  mObserverList.Broadcast(aType);
}

void
FMRadioChild::AddObserver(FMRadioEventObserver* aObserver)
{
  mObserverList.AddObserver(aObserver);
}

void
FMRadioChild::RemoveObserver(FMRadioEventObserver* aObserver)
{
  mObserverList.RemoveObserver(aObserver);
}

void
FMRadioChild::SendRequest(FMRadioReplyRunnable* aReplyRunnable,
                          FMRadioRequestArgs aArgs)
{
  PFMRadioRequestChild* childRequest = new FMRadioRequestChild(aReplyRunnable);
  SendPFMRadioRequestConstructor(childRequest, aArgs);
}

bool
FMRadioChild::RecvNotifyFrequencyChanged(const double& aFrequency)
{
  mFrequency = aFrequency;
  NotifyFMRadioEvent(FrequencyChanged);
  return true;
}

bool
FMRadioChild::RecvNotifyEnabledChanged(const bool& aEnabled,
                                       const double& aFrequency)
{
  mEnabled = aEnabled;
  mFrequency = aFrequency;
  NotifyFMRadioEvent(EnabledChanged);
  return true;
}

bool
FMRadioChild::RecvNotifyRDSChanged(const uint16_t& aPi, const uint8_t& aEcc,
                                   const nsString& aPs, const nsString& aRt)
{
  if (mPi != aPi) {
    mPi = aPi;
    NotifyFMRadioEvent(PiChanged);
  }

  if (mEcc != aEcc) {
    mEcc = aEcc;
    NotifyFMRadioEvent(EccChanged);
  }

  if (mPs != aPs) {
    mPs = aPs;
    NotifyFMRadioEvent(PsChanged);
  }

  if (mRt != aRt) {
    mRt = aRt;
    NotifyFMRadioEvent(RtChanged);
  }

  return true;
}

bool
FMRadioChild::Recv__delete__()
{
  return true;
}

PFMRadioRequestChild*
FMRadioChild::AllocPFMRadioRequestChild(const FMRadioRequestArgs& aArgs)
{
  MOZ_CRASH();
  return nullptr;
}

bool
FMRadioChild::DeallocPFMRadioRequestChild(PFMRadioRequestChild* aActor)
{
  delete aActor;
  return true;
}

void
FMRadioChild::EnableAudio(bool aAudioEnabled)
{
  SendEnableAudio(aAudioEnabled);
}

// static
FMRadioChild*
FMRadioChild::Singleton()
{
  MOZ_ASSERT(XRE_GetProcessType() != GeckoProcessType_Default);
  MOZ_ASSERT(NS_IsMainThread());

  if (!sFMRadioChild) {
    sFMRadioChild = new FMRadioChild();
  }

  return sFMRadioChild;
}

END_FMRADIO_NAMESPACE


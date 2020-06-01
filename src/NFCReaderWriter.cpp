// #############################################################################
// ###                                                                       ###
// ### NXP PN7150 Driver                                                     ###
// ###                                                                       ###
// ### https://github.com/Strooom/PN7150                                     ###
// ### Author(s) : Pascal Roobrouck - @strooom                               ###
// ### License : https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode ###
// ###                                                                       ###
// #############################################################################


#include "NFCReaderWriter.h"
#include "NCI.h"


NFCReaderWriter::NFCReaderWriter(NCI &theNCI) : theNCI(theNCI), theState(ReaderWriterState::initializing)
    {
    }

void NFCReaderWriter::initialize()
    {
    theNCI.initialize();								// initialize the NCI stateMachine and other. Will in its turn initialize the HW interface
    theState = ReaderWriterState::initializing;
#if DEBUGLEVEL > 0
    Serial.println("NFC ReaderWriter initialised");
#endif
    }

void NFCReaderWriter::run()
    {
    theNCI.run();
    switch (theState)
        {
        case ReaderWriterState::initializing:
            {
            switch (theNCI.getState())
                {
                case NciState::RfIdleCmd:
                    {
                    theNCI.activate();
                    theState = ReaderWriterState::noTagPresent;
#if DEBUGLEVEL > 0
                    Serial.println("Polling activated");
#endif
                    }
                break;
                default:
                    break;
                }
            }
        break;

        case ReaderWriterState::noTagPresent:
            {
            switch (theNCI.getState())
                {
                case NciState::RfWaitForHostSelect:
                    {
                    theState = ReaderWriterState::multipleTagsPresent;
#if DEBUGLEVEL > 0
                    uint8_t nmbrTags = theNCI.getNmbrOfTags();
                    Serial.print(nmbrTags);
                    Serial.println(" Tags detected");
					Tag* tmpTag = nullptr;
                    for (uint8_t index = 0; index < nmbrTags; index++)
                        {

                        Serial.print("  Tag[");
                        Serial.print(index);
                        Serial.print("] : ");
                        tmpTag = theNCI.getTag(index);					
                        tmpTag->print();
                        Serial.println("");
                        }
#endif
                    }
                break;
                case NciState::RfPollActive:
                    {
                    theState = ReaderWriterState::singleTagPresent;
#if DEBUGLEVEL > 0
                    Serial.println("Single Tag detected");
                    Tag* tmpTag = nullptr;
                    Serial.print("  Tag[0] : ");
                    tmpTag = theNCI.getTag(0);
                    tmpTag->print();
                    Serial.println("");
#endif
                    }
                break;
                }
            }
        break;

        case ReaderWriterState::singleTagPresent:
            {
            switch (theNCI.getState())
                {
                case NciState::RfIdleCmd:
                    theNCI.activate();
                    break;

                case NciState::RfDiscovery:
                    if (theNCI.getTagsPresentStatus() == TagsPresentStatus::noTagsPresent)
                        {
                        theState = ReaderWriterState::noTagPresent;
#if DEBUGLEVEL > 0
                        Serial.println("Tag removed");
#endif
                        }
                    break;

                case NciState::RfWaitForHostSelect:
                    theNCI.deActivate(NciRfDeAcivationMode::IdleMode);
                    break;

                case NciState::RfPollActive:
                    theNCI.deActivate(NciRfDeAcivationMode::IdleMode);
                    break;
                }
            }
        break;

        case ReaderWriterState::multipleTagsPresent:
            {
            switch (theNCI.getState())
                {
                case NciState::RfIdleCmd:
                    theNCI.activate();
                    break;

                case NciState::RfDiscovery:
                    if (theNCI.getTagsPresentStatus() == TagsPresentStatus::noTagsPresent)
                        {
                        theState = ReaderWriterState::noTagPresent;
#if DEBUGLEVEL > 0
                        Serial.println("Tags removed");
#endif
                        }
                    break;

                case NciState::RfWaitForHostSelect:
                    theNCI.deActivate(NciRfDeAcivationMode::IdleMode);
                    break;

                case NciState::RfPollActive:
                    theNCI.deActivate(NciRfDeAcivationMode::IdleMode);
                    break;
                }
            }
        break;
        }

    if (theNCI.getState() == NciState::Error)
        {
#if DEBUGLEVEL > 0
        Serial.println("Error : Re-initializing NCI");
#endif
        theNCI.initialize();				// initialize the NCI stateMachine and other. Will in its turn initialize the HW interface
        }
    }

ReaderWriterState NFCReaderWriter::getState() const
{
    return theState;
}



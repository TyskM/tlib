#pragma once

#include <SDL2/SDL_events.h>
#include <string>

const std::string SDLEventToStr(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_FIRSTEVENT:                      { return "SDL_FIRSTEVENT";                   }
    case SDL_QUIT:                            { return "SDL_QUIT";                         }
    case SDL_APP_TERMINATING:                 { return "SDL_APP_TERMINATING";              }
    case SDL_APP_LOWMEMORY:                   { return "SDL_APP_LOWMEMORY";                }
    case SDL_APP_WILLENTERBACKGROUND:         { return "SDL_APP_WILLENTERBACKGROUND";      }
    case SDL_APP_DIDENTERBACKGROUND:          { return "SDL_APP_DIDENTERBACKGROUND";       }
    case SDL_APP_WILLENTERFOREGROUND:         { return "SDL_APP_WILLENTERFOREGROUND";      }
    case SDL_APP_DIDENTERFOREGROUND:          { return "SDL_APP_DIDENTERFOREGROUND";       }
    case SDL_LOCALECHANGED:                   { return "SDL_LOCALECHANGED";                }
    case SDL_DISPLAYEVENT:                    { return "SDL_DISPLAYEVENT";                 }
    case SDL_WINDOWEVENT:                     { return "SDL_WINDOWEVENT";                  }
    case SDL_SYSWMEVENT:                      { return "SDL_SYSWMEVENT";                   }
    case SDL_KEYDOWN:                         { return "SDL_KEYDOWN";                      }
    case SDL_KEYUP:                           { return "SDL_KEYUP";                        }
    case SDL_TEXTEDITING:                     { return "SDL_TEXTEDITING";                  }
    case SDL_TEXTINPUT:                       { return "SDL_TEXTINPUT";                    }
    case SDL_KEYMAPCHANGED:                   { return "SDL_KEYMAPCHANGED";                }
    case SDL_TEXTEDITING_EXT:                 { return "SDL_TEXTEDITING_EXT";              }
    case SDL_MOUSEMOTION:                     { return "SDL_MOUSEMOTION";                  }
    case SDL_MOUSEBUTTONDOWN:                 { return "SDL_MOUSEBUTTONDOWN";              }
    case SDL_MOUSEBUTTONUP:                   { return "SDL_MOUSEBUTTONUP";                }
    case SDL_MOUSEWHEEL:                      { return "SDL_MOUSEWHEEL";                   }
    case SDL_JOYAXISMOTION:                   { return "SDL_JOYAXISMOTION";                }
    case SDL_JOYBALLMOTION:                   { return "SDL_JOYBALLMOTION";                }
    case SDL_JOYHATMOTION:                    { return "SDL_JOYHATMOTION";                 }
    case SDL_JOYBUTTONDOWN:                   { return "SDL_JOYBUTTONDOWN";                }
    case SDL_JOYBUTTONUP:                     { return "SDL_JOYBUTTONUP";                  }
    case SDL_JOYDEVICEADDED:                  { return "SDL_JOYDEVICEADDED";               }
    case SDL_JOYDEVICEREMOVED:                { return "SDL_JOYDEVICEREMOVED";             }
    case SDL_JOYBATTERYUPDATED:               { return "SDL_JOYBATTERYUPDATED";            }
    case SDL_CONTROLLERAXISMOTION:            { return "SDL_CONTROLLERAXISMOTION";         }
    case SDL_CONTROLLERBUTTONDOWN:            { return "SDL_CONTROLLERBUTTONDOWN";         }
    case SDL_CONTROLLERBUTTONUP:              { return "SDL_CONTROLLERBUTTONUP";           }
    case SDL_CONTROLLERDEVICEADDED:           { return "SDL_CONTROLLERDEVICEADDED";        }
    case SDL_CONTROLLERDEVICEREMOVED:         { return "SDL_CONTROLLERDEVICEREMOVED";      }
    case SDL_CONTROLLERDEVICEREMAPPED:        { return "SDL_CONTROLLERDEVICEREMAPPED";     }
    case SDL_CONTROLLERTOUCHPADDOWN:          { return "SDL_CONTROLLERTOUCHPADDOWN";       }
    case SDL_CONTROLLERTOUCHPADMOTION:        { return "SDL_CONTROLLERTOUCHPADMOTION";     }
    case SDL_CONTROLLERTOUCHPADUP:            { return "SDL_CONTROLLERTOUCHPADUP";         }
    case SDL_CONTROLLERSENSORUPDATE:          { return "SDL_CONTROLLERSENSORUPDATE";       }
    case SDL_FINGERDOWN:                      { return "SDL_FINGERDOWN";                   }
    case SDL_FINGERUP:                        { return "SDL_FINGERUP";                     }
    case SDL_FINGERMOTION:                    { return "SDL_FINGERMOTION";                 }
    case SDL_DOLLARGESTURE:                   { return "SDL_DOLLARGESTURE";                }
    case SDL_DOLLARRECORD:                    { return "SDL_DOLLARRECORD";                 }
    case SDL_MULTIGESTURE:                    { return "SDL_MULTIGESTURE";                 }
    case SDL_CLIPBOARDUPDATE:                 { return "SDL_CLIPBOARDUPDATE";              }
    case SDL_DROPFILE:                        { return "SDL_DROPFILE";                     }
    case SDL_DROPTEXT:                        { return "SDL_DROPTEXT";                     }
    case SDL_DROPBEGIN:                       { return "SDL_DROPBEGIN";                    }
    case SDL_DROPCOMPLETE:                    { return "SDL_DROPCOMPLETE";                 }
    case SDL_AUDIODEVICEADDED:                { return "SDL_AUDIODEVICEADDED";             }
    case SDL_AUDIODEVICEREMOVED:              { return "SDL_AUDIODEVICEREMOVED";           }
    case SDL_SENSORUPDATE:                    { return "SDL_SENSORUPDATE";                 }
    case SDL_RENDER_TARGETS_RESET:            { return "SDL_RENDER_TARGETS_RESET";         }
    case SDL_RENDER_DEVICE_RESET:             { return "SDL_RENDER_DEVICE_RESET";          }
    case SDL_POLLSENTINEL:                    { return "SDL_POLLSENTINEL";                 }
    case SDL_USEREVENT:                       { return "SDL_USEREVENT";                    }
    case SDL_LASTEVENT:                       { return "SDL_LASTEVENT";                    }
    default: return "Unknown";
    }
}
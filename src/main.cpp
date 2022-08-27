#include "olcPixelGameEngine.h"
#include "olcSoundWaveEngine.h"

#include "AssetManager.h"
using Assets = AssetManager;

// Disable the console window on Windows.
#define ConsoleWindowDisabled
#if defined( ConsoleWindowDisabled )
#if defined( _MSC_VER )
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
#endif

namespace Palette
{
    struct Palette
    {
        olc::Pixel Darkest = { 0,0,0 };
        olc::Pixel Dark = { 157,157,157 };
        olc::Pixel Neutral = { 184,184,184 };
        olc::Pixel Bright = { 255,255,255 };
    };

    struct Amber : Palette
    {
        Amber()
        {
            Darkest = { 0,0,0 };
            Dark = { 157,105,0 };
            Neutral = { 184,123,0 };
            Bright = { 255,170,0 };
        }        
    } Amber;

    struct Blue : Palette
    {
        Blue()
        {
            Darkest = { 0,0,0 };
            Dark = { 0,105,157 };
            Neutral = { 0,123,184 };
            Bright = { 0,170,255 };
        }        
    } Blue;
}

class PourHonor : public olc::PixelGameEngine
{
public:
    PourHonor()
    {
        sAppName = "PourHonor";
    }

    enum GameState
    {
	    Title = 0, // Title card that waits for user input. (Press Enter to begin)
        Intro, // Intro that is played after the title card. (skip using Spacebar)

        Observe, // Stare out the window and try to dicern what the weather will be.
        Predict, // Submit predictions to the computer.
        Presentation, // Your predictions are presented on the television.
        Reality, // Were you indeed correct?
        Consequences, // The senate will decide your fate.

        Lose, // You're fired, your reputation is in the dumps.
        Win, // You're the greatest weatherman that has ever existed, good on you buddy!

        Maximum
    } State = Title;

    Palette::Palette Palette = Palette::Amber;
    uint8_t PaletteIndex = 0;

    bool OnUserCreate() override
    {
        WaveEngine.InitialiseAudio();
        Assets::LoadGraphic( "background", "assets/gfx/space.png" );

        Assets::LoadSound( "bg-music", "assets/sounds/TheHaedooLow11250.wav" );
        Assets::LoadSound( "laser", "assets/sounds/Laser_Shoot11.wav" );
        Assets::LoadSound( "explosion", "assets/sounds/Explosions1.wav" );
        Assets::LoadSound( "lose", "assets/sounds/lose9.wav" );
        Assets::LoadSound( "thruster", "assets/sounds/thruster.wav" );

        BackgroundLayer = CreateLayer();
        EnableLayer( BackgroundLayer, true );

        SetDrawTarget( BackgroundLayer );
        // DrawSprite( 0, 0, Assets::GetSprite( "background" ) );
        SetDrawTarget( nullptr );

        // auto Wave = WaveEngine.PlayWaveform( Assets::GetSound( "bg-music" ), true );

        return true;
    }

    bool UpdateState()
    {
        switch( State )
        {
        case Title:
            UpdateTitle();
            return true;
        case Intro:
            UpdateIntro();
            return true;
        case Observe:
            UpdateObserve();
            return true;
        case Predict:
            UpdatePredict();
            return true;
        case Presentation:
            UpdatePresentation();
            return true;
        case Reality:
            UpdateReality();
            return true;
        case Consequences:
            UpdateConsequences();
            return true;
        case Lose:
            UpdateLose();
            return true;
        case Win:
            UpdateWin();
            return true;
        case Maximum: break;
        default:;
        }

        return false;
    }

    bool OnUserUpdate( float DeltaTime ) override
    {
        Clear( Palette.Darkest );

        const auto ValidState = UpdateState();
        if( !ValidState )
        {
            DrawString( 0, 0, "Current game state has not been implemented.", Palette.Bright );
        }

        if( GetKey( olc::ENTER ).bReleased )
        {
            State = static_cast<GameState>( State + 1 );
            if( State >= Maximum )
            {
                // Unknown state, return to title screen.
                State = Title;
            }
        }

        if( GetKey( olc::NP_ADD ).bReleased )
        {
            PaletteIndex++;
            PaletteIndex = PaletteIndex % 2;

            switch( PaletteIndex )
            {
            case 0:
                Palette = Palette::Amber;
                break;
            case 1:
                Palette = Palette::Blue;
                break;
            default:
                break;
            }
        }

        return !GetKey( olc::ESCAPE ).bPressed;
    }

    void UpdateTitle()
    {
        DrawString( 0, 0, "Pour Honor Title Screen", Palette.Bright );
    }

    void UpdateIntro()
    {
        DrawString( 0, 0, "Woah, I'm a weatherman? Neat...", Palette.Bright );
    }

    void UpdateObserve()
    {
        DrawString( 0, 0, "Hmm.. That cloud looks kinda suspicious.", Palette.Bright );
    }

    void UpdatePredict()
    {
        DrawString( 0, 0, "Computer!\nI'd like to report this weather thing I just saw.", Palette.Bright );
    }

    void UpdatePresentation()
    {
        DrawString( 0, 0, "I love seeing my predictions on TV.", Palette.Bright );
    }

    void UpdateReality()
    {
        DrawString( 0, 0, "This looks like what I expected.. Or wait? What's that!?", Palette.Bright );
    }

    void UpdateConsequences()
    {
        DrawString( 0, 0, "Well, well, well... If it isn't the consequences of my actions.", Palette.Bright );
    }

    void UpdateLose()
    {
        DrawString( 0, 0, "You're fired, your reputation has taken a nosedive.", Palette.Bright );
    }

    void UpdateWin()
    {
        DrawString( 0, 0, "You're the greatest weatherman that has ever existed, good on you buddy!", Palette.Bright );
    }

    olc::sound::WaveEngine WaveEngine;
    int BackgroundLayer = -1;
};


int main()
{
    PourHonor Game;
    if( Game.Construct( 320, 180, 2, 2 ) )
        Game.Start();

    return 0;
}


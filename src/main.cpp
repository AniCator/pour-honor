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

class PourHonor : public olc::PixelGameEngine
{
public:
    PourHonor()
    {
        sAppName = "PourHonor";
    }

    enum GameState
    {
	    Title, // Title card that waits for user input. (Press Enter to begin)
        Intro, // Intro that is played after the title card. (skip using Spacebar)

        Observe, // Stare out the window and try to dicern what the weather will be.
        Predict, // Submit predictions to the computer.
        Presentation, // Your predictions are presented on the television.
        Reality, // Were you indeed correct?
        Consequences, // The senate will decide your fate.

        Lose, // You're fired, your reputation is in the dumps.
        Win, // You're the greatest weatherman that has ever existed, good on you buddy!
    } State;

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
        DrawSprite( 0, 0, Assets::GetSprite( "background" ) );
        SetDrawTarget( nullptr );

        auto Wave = WaveEngine.PlayWaveform( Assets::GetSound( "bg-music" ), true );

        return true;
    }

    bool OnUserUpdate( float DeltaTime ) override
    {
        Clear( olc::BLANK );

        DrawLine( 0, 0, 320, 180 );

        return !GetKey( olc::ESCAPE ).bPressed;
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


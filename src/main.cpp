#include "olcPixelGameEngine.h"
#include "olcSoundWaveEngine.h"
#include "AssetManager.h"

using Assets = AssetManager;

class PourHonor : public olc::PixelGameEngine
{
public:
    PourHonor()
    {
        sAppName = "PourHonor";
    }

private:
    olc::sound::WaveEngine WaveEngine;
    int BackgroundLayer = -1;

public:
    bool OnUserCreate() override
    {
        WaveEngine.InitialiseAudio();
        Assets::LoadGraphic( "background", "assets/gfx/space.png" );

        Assets::LoadSound( "bg-music", "assets/sounds/bg-music.wav" );
        Assets::LoadSound( "laser", "assets/sounds/Laser_Shoot11.wav" );
        Assets::LoadSound( "explosion", "assets/sounds/Explosions1.wav" );
        Assets::LoadSound( "lose", "assets/sounds/lose9.wav" );
        Assets::LoadSound( "thruster", "assets/sounds/thruster.wav" );

        BackgroundLayer = CreateLayer();
        EnableLayer( BackgroundLayer, true );

        SetDrawTarget( BackgroundLayer );
        DrawSprite( 0, 0, Assets::GetSprite( "background" ) );
        SetDrawTarget( nullptr );

        WaveEngine.PlayWaveform( Assets::GetSound( "bg-music" ), true );

        return true;
    }

    // Called by olcConsoleGameEngine
    bool OnUserUpdate( float DeltaTime ) override
    {
        Clear( olc::BLANK );

        return !GetKey( olc::ESCAPE ).bPressed;
    }
};


int main()
{
    PourHonor game;
    if( game.Construct( 320, 180, 4, 4 ) )
        game.Start();

    return 0;
}


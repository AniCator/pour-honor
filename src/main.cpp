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

constexpr float TickRate = 1.0f / 30.0f;

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
    GameState PreviousState = State;
    GameState MomentaryState = State;
    bool StateJustChanged = false;

    uint8_t Reputation = 50;

    Palette::Palette Palette = Palette::Amber;
    uint8_t PaletteIndex = 0;

    float DeltaTime = 1.0f / 60.0f;
    float Time = 0.0f;
    float TimeSinceLastTick = 0.0f;

    float StateChangeTime = 0.0f;
    float TimeSinceStateChange() const
    {
        return Time - StateChangeTime;
    }

    int WeatherLayer = -1;
    int WindowLayer = -1;
    int ComputerLayer = -1;
    int TVLayer = -1;

    olc::Sprite* WeatherSprite = nullptr;

    struct IntroCard
    {
        float Duration = 3.0f;
        std::string Asset;
        std::string Text;

        olc::Sprite* Sprite = nullptr;
    };

    std::vector<IntroCard> Cards = {
        {4.0f,"slide1", "The life of a weatherman."},
        {5.0f,"slide2", "The life of a weatherman...\nIs filled with worry."},
        {2.0f,"slide1", "Developing a keen weather sense."},
        {4.0f,"slide1", "Developing a keen weather sense..\nIs paramount."},
        {2.0f,"slide1", "Wake up.."},
        {4.0f,"slide1", "Wake up..\nIt's time to gaze."},
        {4.0f,"clouds", "Watch the clouds."},
        {4.0f,"computer", "Then press Enter\nto submit your prediction."},
        {2.0f,"slide1", "Take your time."},
        {4.0f,"slide1", "Take your time..\nBut not too much time."},
        {0.2f,"slide1", "Your "},
        {0.75f,"slide1", "Your reputation"},
        {0.25f,"slide1", "Your reputation is"},
        {0.25f,"slide1", "Your reputation is at"},
        {4.0f,"slide1", "Your reputation is at stake."}
    };

    bool OnUserCreate() override
    {
        WaveEngine.InitialiseAudio( 44100, 2, 8, 2048 );

        // Load sounds.
        Assets::LoadSound( "music", "assets/sounds/cator_weather_channel.wav" );
        Assets::LoadSound( "music_intro", "assets/sounds/cator_weather_intro.wav" );
        Assets::LoadSound( "computer_idle", "assets/sounds/computer_idle.wav" );
        Assets::LoadSound( "computer_prediction", "assets/sounds/computer_prediction.wav" );
        Assets::LoadSound( "speech", "assets/sounds/speech.wav" );
        Assets::LoadSound( "reality_sets_in", "assets/sounds/reality_sets_in.wav" );

        // Load graphics.
        Assets::LoadGraphic( "clouds", "assets/sprites/clouds.png" );
        Assets::LoadGraphic( "weather", "assets/sprites/weather.png" );
        Assets::LoadGraphic( "window", "assets/sprites/window.png" );
        Assets::LoadGraphic( "computer", "assets/sprites/computer.png" );
        Assets::LoadGraphic( "tv", "assets/sprites/tv.png" );

        // Load and configure the intro sprites.
        for( auto& Card : Cards )
        {
            Assets::LoadGraphic( Card.Asset, "assets/sprites/" + Card.Asset + ".png" );
            Card.Sprite = Assets::GetSprite( Card.Asset );
        }

        // Configure remaining sprites and layers.
        WeatherSprite = Assets::GetSprite( "weather" );

        TVLayer = CreateLayer();
        SetDrawTarget( TVLayer );
        DrawSprite( 0, 0, Assets::GetSprite( "tv" ) );

        ComputerLayer = CreateLayer();
        SetDrawTarget( ComputerLayer );
        DrawSprite( 0, 0, Assets::GetSprite( "computer" ) );

        WindowLayer = CreateLayer();
        SetDrawTarget( WindowLayer );
        DrawSprite( 0, 0, Assets::GetSprite( "window" ) );

        WeatherLayer = CreateLayer();

        // We're no longer using the draw targets.
        SetDrawTarget( nullptr );

        return true;
    }

    olc::sound::PlayingWave BackgroundMusic;

    void DisableLayers()
    {
        EnableLayer( WeatherLayer, false );
        EnableLayer( WindowLayer, false );
        EnableLayer( ComputerLayer, false );
        EnableLayer( TVLayer, false );
    }

    bool UpdateState()
    {
        DisableLayers();

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

    bool AnyKey() const
    {
        for( size_t Index = 0; Index < 97; Index++ )
        {
            if( GetKey( static_cast<olc::Key>( Index ) ).bReleased )
                return true;
        }

        return false;
    }

    bool OnUserUpdate( float DeltaTime ) override
    {
        this->DeltaTime = DeltaTime;
        Time += DeltaTime;

        /*TimeSinceLastTick += DeltaTime;
        if( TimeSinceLastTick < TickRate )
            return !GetKey( olc::ESCAPE ).bPressed;

        TimeSinceLastTick = 0.0f;*/

        if( State != MomentaryState )
        {
            StateChangeTime = Time;
            PreviousState = MomentaryState;
            MomentaryState = State;

            StateJustChanged = true;
        }
        else
        {
            StateJustChanged = false;
        }

        Clear( olc::BLANK );

        const auto ValidState = UpdateState();
        if( !ValidState )
        {
            DrawString( 0, 0, "Current game state has not been implemented.", Palette.Bright );
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

    bool HasPlayedBackgroundSoundAtLeastOnce = false;
    olc::sound::PlayingWave BackgroundSound;

    void PlayBackgroundSound( olc::sound::Wave* Pointer, const bool Loop = true )
    {
        if( !Pointer )
            return;

        if( HasPlayedBackgroundSoundAtLeastOnce )
        {
            WaveEngine.StopWaveform( BackgroundSound );
        }

        BackgroundSound = WaveEngine.PlayWaveform( Pointer, Loop );
        HasPlayedBackgroundSoundAtLeastOnce = Loop;
    }

    void UpdateTitle()
    {
        Clear( Palette.Darkest );
        DrawString( 130, 60, "Pour Honor", Palette.Bright );
        DrawLine( 125, 70, 210, 70, Palette.Dark );
        DrawString( 90, 100, "Press Enter To Start", Palette.Bright );

        if( GetKey( olc::ENTER ).bReleased )
        {
            State = Intro;
        }
    }

    size_t CardIndex = 0;
    float CardStartTime = -1.0f;

    void RenderCard( const IntroCard& Card )
    {
        if( Card.Sprite )
        {
            DrawSprite( 0, 0, Card.Sprite );
        }

        if( !Card.Text.empty() )
        {
            auto RectangleColor = Palette.Darkest;
            RectangleColor.a = 200;

            SetPixelMode( olc::Pixel::ALPHA );
            FillRect( 0, 149, 320, 31, RectangleColor );
            SetPixelMode( olc::Pixel::NORMAL );
            DrawLine( 0, 148, 320, 148, Palette.Bright );
            DrawString( 5, 150, Card.Text, Palette.Neutral );
        }
    }

    void UpdateIntro()
    {
        if( StateJustChanged )
        {
            CardIndex = 0;
            CardStartTime = TimeSinceStateChange();

            BackgroundMusic = WaveEngine.PlayWaveform( Assets::GetSound( "music_intro" ), true );
        }

        // Check if we can still be in this state.
        if( Cards.empty() || CardIndex >= Cards.size() )
        {
            State = Observe;
            return;
        }

        Clear( Palette.Darkest );

        const auto& Card = Cards[CardIndex];
        RenderCard( Card );

        const auto CardDeltaTime = TimeSinceStateChange() - CardStartTime;
        const auto WantsSkip = ( CardIndex > 0 && GetKey( olc::ENTER ).bReleased ) || GetKey( olc::SPACE ).bReleased;
        if( WantsSkip || CardDeltaTime > Card.Duration )
        {
            CardIndex++;
            CardStartTime = TimeSinceStateChange();
        }
    }

    void RenderWeatherLayer()
    {
        SetDrawTarget( WeatherLayer );
        if( WeatherSprite )
        {
            // Clear to sky background.
            Clear( olc::Pixel( 172, 115, 0 ) );
            const auto WrapAround = WeatherSprite->width - 320;
            const auto Offset = ( static_cast<int32_t>( TimeSinceStateChange() * -10.0f ) + 320 ) % WrapAround;
            DrawSprite( Offset, 0, WeatherSprite );
        }
        SetDrawTarget( nullptr );
    }

    void UpdateObserve()
    {
        DrawString( 0, 0, "Your reputation: " + std::to_string( Reputation ), Palette.Bright );

        if( TimeSinceStateChange() < 3.0f )
        {
            DrawString( 0, 10, "Hmm.. That cloud looks kinda suspicious.", Palette.Bright );
        }

        RenderWeatherLayer();

        EnableLayer( WeatherLayer, true );
        EnableLayer( WindowLayer, true );

        DrawString( 0, 170, "You've been staring for " + std::to_string( static_cast<int32_t>( TimeSinceStateChange() ) ) + " seconds.", Palette.Bright );

        if( GetKey( olc::ENTER ).bReleased )
        {
            State = Predict;
        }

        if( !StateJustChanged )
            return;

        WaveEngine.StopWaveform( BackgroundMusic );
        BackgroundMusic = WaveEngine.PlayWaveform( Assets::GetSound( "music" ), true );
    }

    olc::sound::PlayingWave InteractionSound;
    void UpdatePredict()
    {
        EnableLayer( ComputerLayer, true );
        DrawString( 0, 0, "Computer!\nI'd like to report..\nThis weather thing I just saw.", Palette.Bright );

        if( AnyKey() )
        {
            InteractionSound = WaveEngine.PlayWaveform( Assets::GetSound( "computer_prediction" ) );
        }

        if( GetKey( olc::ENTER ).bReleased )
        {
            State = Presentation;
        }

        if( !StateJustChanged )
            return;

        PlayBackgroundSound( Assets::GetSound( "computer_idle" ) );
    }

    void UpdatePresentation()
    {
        EnableLayer( TVLayer, true );
        DrawString( 0, 0, "The predictions are in...", Palette.Bright );

        if( AnyKey() )
        {
            InteractionSound = WaveEngine.PlayWaveform( Assets::GetSound( "speech" ) );
        }

        if( TimeSinceStateChange() > 5.0f )
        {
            State = Reality;
        }

        if( !StateJustChanged )
            return;

        PlayBackgroundSound( Assets::GetSound( "speech" ) );
    }

    void UpdateReality()
    {
        const auto Duration = TimeSinceStateChange();
        EnableLayer( WeatherLayer, true );
        SetLayerOffset( WeatherLayer, Duration * 0.1 - 1.0f, 0.0f );
        DrawString( 0, 0, "This looks like what I expected..\nOr wait? What's that!?", Palette.Bright );

        if( Duration > 7.0f )
        {
            State = Consequences;
        }

        if( !StateJustChanged )
            return;

        WaveEngine.StopWaveform( BackgroundMusic );
        PlayBackgroundSound( Assets::GetSound( "reality_sets_in" ), false );
    }

    void UpdateConsequences()
    {
        DrawString( 0, 0, "Well, well, well...\nIf it isn't the consequences\nof my actions.", Palette.Bright );

        if( TimeSinceStateChange() > 6.0f )
        {
            State = Lose;
        }
    }

    void UpdateLose()
    {
        DrawString( 0, 0, "You're fired...\nYour reputation has taken a nosedive.", Palette.Bright );

        if( AnyKey() || TimeSinceStateChange() > 10.0f )
        {
            State = Title;
        }
    }

    void UpdateWin()
    {
        DrawString( 0, 0, "You're the greatest weatherman\nthat has ever existed.\n\nGood on you buddy!", Palette.Bright );

        if( AnyKey() || TimeSinceStateChange() > 10.0f )
        {
            State = Title;
        }
    }

    olc::sound::WaveEngine WaveEngine;
};


int main()
{
    PourHonor Game;
    if( Game.Construct( 320, 180, 2, 2, false, true ) )
        Game.Start();

    return 0;
}


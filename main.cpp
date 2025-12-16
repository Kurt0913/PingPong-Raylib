#include "include/raylib.h"
#include <stdio.h> // Needed for sprintf/TextFormat

// --- 1. DEFINE STATES ---
typedef enum GameState { 
    SPLASH, 
    COUNTDOWN, // New State
    GAMEPLAY, 
    ENDING 
} GameState;

struct Ball {
    float x, y;
    float speedX, speedY;
    float radius;
};

struct Paddle {
    float x, y;
    float width, height;
    float speed;
};

// --- HELPER FUNCTION ---
bool CheckCollisionBallPaddle(Ball b, Paddle p) {
    if (b.x + b.radius >= p.x &&            
        b.x - b.radius <= p.x + p.width &&  
        b.y + b.radius >= p.y &&            
        b.y - b.radius <= p.y + p.height) { 
            return true;
    }
    return false;
}

int main() {
    InitWindow(800, 600, "PINGPONG");
    SetTargetFPS(60);
    InitAudioDevice();

    // --- VARIABLES ---
    GameState currentState = SPLASH; 

    // Scores & Timers
    int player1Score = 0;
    int player2Score = 0;
    
    float splashTimer = 0.0f;
    float countdownTimer = 3.0f; // 3 Seconds to get ready
    float gameTimer = 60.0f;
    
    int alpha = 0;
    int promptAlpha = 0;
    int winner = 0;
    
    bool isStarting = false;     // Tracks if we hit space and are transitioning
    float flickerTimer = 0.0f;   // How long the flicker lasts
    Color textColor = GRAY;      // To change color when hit

    // Objects
    Ball ball;
    ball.x = 400; ball.y = 300;
    ball.speedX = 5; ball.speedY = 5;
    ball.radius = 15;

    Paddle leftPaddle;
    leftPaddle.x = 50; leftPaddle.y = 250;
    leftPaddle.width = 20; leftPaddle.height = 100;
    leftPaddle.speed = 5;

    Paddle rightPaddle;
    rightPaddle.x = 730; 
    rightPaddle.y = 250;
    rightPaddle.width = 20; rightPaddle.height = 100;
    rightPaddle.speed = 5;

    // Load Sounds
    Sound startSound = LoadSound("start.wav");
    Sound wallSound = LoadSound("wall.wav");
    Music gameMusic = LoadMusicStream("Background.mp3");
    Music gameplayMusic = LoadMusicStream("gameplay.mp3");

    // --- GAME LOOP ---
    while (!WindowShouldClose()) {
        
        UpdateMusicStream(gameMusic);
        UpdateMusicStream(gameplayMusic);
        //      UPDATE PHASE (LOGIC)
        switch (currentState) {

            case SPLASH:
                splashTimer += GetFrameTime();

                // --- BEHAVIOR A: WAITING FOR INPUT ---
                if (!isStarting) {
                    // 1. Play Music
                    if (!IsMusicStreamPlaying(gameMusic)) {
                        PlayMusicStream(gameMusic);
                        SetMusicVolume(gameMusic, 0.7f);
                    }

                    // 2. Fade In Logic (Title)
                    if (splashTimer < 2.0f) {
                        alpha = (int)((splashTimer / 2.0f) * 255);
                        promptAlpha = 0; // Invisible while title fades in
                    } else {
                        alpha = 255; // Title is fully solid now
                        
                        // Pulse the prompt slowly (Breathing effect)
                        if ((int)(GetTime() * 5) % 2 == 0) promptAlpha = 255;
                        else promptAlpha = 100; 
                    }
                    
                    textColor = GRAY; // Reset color

                    // 3. Detect Input
                    if (splashTimer > 2.0f && IsKeyPressed(KEY_SPACE)) {
                        isStarting = true;          
                        StopMusicStream(gameMusic); 
                        PlaySound(startSound);      
                        flickerTimer = 0.0f;        
                    }
                } 
                
                // --- BEHAVIOR B: FLICKER EFFECT (Starting) ---
                else {
                    flickerTimer += GetFrameTime();

                    // 1. LOCK TITLE SOLID (Fixes the flickering title)
                    alpha = 255; 

                    // 2. FLICKER PROMPT ONLY
                    promptAlpha = GetRandomValue(0, 255);

                    // 3. Flash Colors
                    if (GetRandomValue(0, 1) == 0) textColor = YELLOW;
                    else textColor = WHITE;

                    // 4. End Transition
                    if (flickerTimer > 1.0f) {
                        currentState = COUNTDOWN;
                        countdownTimer = 3.0f;
                        isStarting = false;
                    }
                }
                break;

            case COUNTDOWN:
                // Only subtract time. Do NOT move ball or paddles.
                countdownTimer -= GetFrameTime();
                
                // When time is up, start the game
                if (countdownTimer <= 0) {
                    currentState = GAMEPLAY;
                }
                break;

            case GAMEPLAY:

            if (!IsMusicStreamPlaying(gameplayMusic)) {
                    PlayMusicStream(gameplayMusic);
                    SetMusicVolume(gameplayMusic, 0.5f); // Lower volume so you hear SFX
                }
                // Timer
                gameTimer -= GetFrameTime();
                if (gameTimer <= 0) {
                    currentState = ENDING;
                    if (player1Score > player2Score) winner = 1;
                    else if (player2Score > player1Score) winner = 2;
                    else winner = 0;
                }

                // Movement
                ball.x += ball.speedX;
                ball.y += ball.speedY;

                // Inputs
                if (IsKeyDown(KEY_W)) leftPaddle.y -= leftPaddle.speed;
                if (IsKeyDown(KEY_S)) leftPaddle.y += leftPaddle.speed;
                if (IsKeyDown(KEY_UP)) rightPaddle.y -= rightPaddle.speed;
                if (IsKeyDown(KEY_DOWN)) rightPaddle.y += rightPaddle.speed;

                // Clamping
                if (leftPaddle.y < 0) leftPaddle.y = 0;
                if (leftPaddle.y > 600 - leftPaddle.height) leftPaddle.y = 600 - leftPaddle.height;
                if (rightPaddle.y < 0) rightPaddle.y = 0;
                if (rightPaddle.y > 600 - rightPaddle.height) rightPaddle.y = 600 - rightPaddle.height;

                // Collisions (Walls)
                if (ball.y + ball.radius >= 600 || ball.y - ball.radius <= 0) 
                    ball.speedY *= -1;

                // Scoring
                if (ball.x >= 800) { 
                    player1Score++;
                    ball.x = 400; ball.y = 300; ball.speedX = 5; ball.speedY = 5;
                    PlaySound(wallSound);
                }
                if (ball.x <= 0) { 
                    player2Score++;
                    ball.x = 400; ball.y = 300; ball.speedX = -5; ball.speedY = 5;
                    PlaySound(wallSound);
                }

                // Collisions (Paddles)
                if (CheckCollisionBallPaddle(ball, leftPaddle)) 
                    ball.speedX *= -1.1f;
                if (CheckCollisionBallPaddle(ball, rightPaddle)) 
                    ball.speedX *= -1.1f;
                break;

            case ENDING:
                StopMusicStream(gameplayMusic);
                // CHANGED: Restart goes to COUNTDOWN first
                if (IsKeyPressed(KEY_ENTER)) {
                    currentState = COUNTDOWN; 
                    countdownTimer = 3.0f; // Reset countdown
                    gameTimer = 60.0f;
                    player1Score = 0;
                    player2Score = 0;
                    ball.x = 400; ball.y = 300; 
                    ball.speedX = 5; ball.speedY = 5;
                    winner = 0;
                }
                break;
        }

        // ============================
        //      DRAW PHASE (RENDER)
        // ============================
        BeginDrawing();
            ClearBackground(BLACK);

            switch (currentState) {
                case SPLASH:
                    DrawText("PINGPONG", 260, 250, 60, Fade(WHITE, (float)alpha/255.0f));

                    if (splashTimer > 2.0f) {
                    DrawText("PRESS SPACE TO START", 270, 450, 20, Fade(textColor, (float)promptAlpha/255.0f));
                    }
                    break;

                case COUNTDOWN:
                    // 1. Draw the game board (Frozen) so players see positions
                    DrawLine(400, 0, 400, 600, Fade(WHITE, 0.3f)); // Dimmed line
                    DrawCircle((int)ball.x, (int)ball.y, ball.radius, WHITE);
                    DrawRectangle((int)leftPaddle.x, (int)leftPaddle.y, (int)leftPaddle.width, (int)leftPaddle.height, WHITE);
                    DrawRectangle((int)rightPaddle.x, (int)rightPaddle.y, (int)rightPaddle.width, (int)rightPaddle.height, WHITE);
                    
                    // 2. Draw the BIG Numbers
                    // logic: if timer is 2.5, (int)timer is 2. We add 1 to show "3"
                    DrawText(TextFormat("%i", (int)countdownTimer + 1), 360, 250, 100, YELLOW);
                    DrawText("GET READY...", 320, 400, 20, LIGHTGRAY);
                    break;

                case GAMEPLAY:
                    DrawLine(400, 0, 400, 600, WHITE);
                    DrawText(TextFormat("Time: %.1f", gameTimer), 350, 550, 20, GREEN);
                    DrawText(TextFormat("%i", player1Score), 200, 20, 80, LIGHTGRAY);
                    DrawText(TextFormat("%i", player2Score), 600, 20, 80, LIGHTGRAY);
                    
                    DrawCircle((int)ball.x, (int)ball.y, ball.radius, WHITE);
                    DrawRectangle((int)leftPaddle.x, (int)leftPaddle.y, (int)leftPaddle.width, (int)leftPaddle.height, WHITE);
                    DrawRectangle((int)rightPaddle.x, (int)rightPaddle.y, (int)rightPaddle.width, (int)rightPaddle.height, WHITE);
                    break;

                case ENDING:
                    DrawText("GAME OVER", 280, 200, 40, RED);
                    if (winner == 1) DrawText("Player 1 Wins!", 300, 260, 30, YELLOW);
                    else if (winner == 2) DrawText("Player 2 Wins!", 300, 260, 30, YELLOW);
                    else DrawText("It's a Draw!", 320, 260, 30, YELLOW);
                    DrawText("Press ENTER to Play Again", 260, 400, 20, WHITE);
                    break;
            }

        EndDrawing();
    }
    UnloadSound(startSound);
    UnloadSound(wallSound);   // Unload sound data
    UnloadMusicStream(gameMusic);
    UnloadMusicStream(gameplayMusic);
    CloseAudioDevice();       // Close the audio system
    CloseWindow();
    return 0;
}
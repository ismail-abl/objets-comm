import pygame
import sys
import random

# ------------------------------
# CONFIG
# ------------------------------
WIDTH = 600
HEIGHT = 400
FPS = 60

PADDLE_WIDTH = 100
PADDLE_HEIGHT = 15
BALL_SIZE = 12

PADDLE_SPEED = 10       # vitesse max (sera modulée par le joystick)
AXIS_DEADZONE = 0.15    # joystick mort

# ------------------------------
# CLASSES
# ------------------------------
class Paddle:
    def __init__(self):
        self.x = WIDTH // 2 - PADDLE_WIDTH // 2
        self.y = HEIGHT - 40
        self.speed = 0

    def update(self, axis_x):
        # Mort de zone
        if abs(axis_x) < AXIS_DEADZONE:
            axis_x = 0

        # vitesse proportionnelle au joystick
        self.speed = axis_x * PADDLE_SPEED

        # déplacement
        self.x += self.speed
        self.x = max(0, min(WIDTH - PADDLE_WIDTH, self.x))

    def draw(self, screen):
        pygame.draw.rect(screen, (0, 200, 255),
                         (self.x, self.y, PADDLE_WIDTH, PADDLE_HEIGHT))


class Ball:
    def __init__(self):
        self.reset()

    def reset(self):
        self.x = WIDTH // 2
        self.y = HEIGHT // 2
        self.dx = random.choice([-4, 4])
        self.dy = -4

    def update(self, paddle: Paddle):
        self.x += self.dx
        self.y += self.dy

        # rebond sur murs latéraux
        if self.x <= 0 or self.x >= WIDTH - BALL_SIZE:
            self.dx *= -1

        # rebond plafond
        if self.y <= 0:
            self.dy *= -1

        # collision paddle
        if (self.y + BALL_SIZE >= paddle.y and
            paddle.x <= self.x <= paddle.x + PADDLE_WIDTH):
            self.dy *= -1

        # balle perdue → reset
        if self.y > HEIGHT:
            self.reset()

    def draw(self, screen):
        pygame.draw.rect(screen, (255, 255, 255),
                         (self.x, self.y, BALL_SIZE, BALL_SIZE))


# ------------------------------
# MAIN LOOP
# ------------------------------
def main():
    pygame.init()
    pygame.joystick.init()

    if pygame.joystick.get_count() == 0:
        print("❌ Aucun joystick HID détecté.")
        print("➡ Lance d'abord : sudo python3 uwb-joystick.py")
        sys.exit()

    js = pygame.joystick.Joystick(0)
    js.init()

    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Ping-Pong UWB Joystick")
    clock = pygame.time.Clock()

    paddle = Paddle()
    ball = Ball()

    while True:
        clock.tick(FPS)

        # événements
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()

        # lecture axe X du joystick HID
        axis_x = js.get_axis(0)   # [-1 .. +1]

        # mise à jour paddle
        paddle.update(axis_x)

        # mise à jour balle
        ball.update(paddle)

        # affichage
        screen.fill((0, 0, 0))
        paddle.draw(screen)
        ball.draw(screen)
        pygame.display.flip()

if __name__ == "__main__":
    main()


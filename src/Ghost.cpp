#include <map>
#include <random>
#include "../include/Ghost.h"
#include "../include/Game.h"

TYPE Ghost::getType() {
    return GHOST;
}

Direction getOppositeDirection(Direction direction) {
    switch (direction) {
        case UP:
            return DOWN;
        case DOWN:
            return UP;
        case LEFT:
            return RIGHT;
        case RIGHT:
            return LEFT;
        default:
            return UP;
    }
}

void Ghost::update() {
    std::map<Direction, SDL_Rect> directions;

    SDL_Rect temp = m_positionRectangle;
    temp.y -= m_movementSpeed;
    directions.insert({UP, temp});
    temp = m_positionRectangle;
    temp.y += m_movementSpeed;
    directions.insert({DOWN, temp});
    temp = m_positionRectangle;
    temp.x -= m_movementSpeed;
    directions.insert({LEFT, temp});
    temp = m_positionRectangle;
    temp.x += m_movementSpeed;
    directions.insert({RIGHT, temp});

    std::map<Direction, SDL_Rect> possibleDirections;
    std::vector<Direction> possibleDirectionsVector;

    //Finds valid move direction
    for (auto &directionPosition : directions) {
        bool didNotCollideWithWall = true;
        for (auto &object : Game::getStationaryGameObjects()) {
            if (object->getType() == WALL) {
                if (SDL_HasIntersection(&directionPosition.second, &object->m_positionRectangle)) {
                    didNotCollideWithWall = false;
                }
            }
        }
        if (didNotCollideWithWall && directionPosition.first != getOppositeDirection(direction)) {
            possibleDirections.insert(directionPosition);
            possibleDirectionsVector.push_back(directionPosition.first);
        }
    }

    if (possibleDirections.empty()) {
        direction = getOppositeDirection(direction);
        m_positionRectangle = directions[direction];
        return;
    }

    //decides which way
    if ((prevDirections != possibleDirectionsVector || possibleDirections.size() > 2)) {
        auto item = possibleDirections.begin();
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<int> dist(0, possibleDirections.size() - 1);
        std::advance(item, dist(mt));

        if (rand() % 10 < difficulty) {
            Direction closestToPlayer = getDirectionToPlayer(possibleDirections);
            direction = closestToPlayer;
            m_positionRectangle = possibleDirections[closestToPlayer];
        } else {
            direction = item->first;
            m_positionRectangle = item->second;
        }
    } else {
        if (switchedToPowerPelletState) {
            direction = getDirectionToPlayer(directions);
            powerPelletState = true;
            switchedToPowerPelletState = false;

            powerPelletStateEndTimer = SDL_AddTimer(5000, &Ghost::powerPelletStateEndCallback, this);
            ghostReviveTimer = SDL_AddTimer(7000, &Ghost::reviveGhostCallback, this);
        }
        m_positionRectangle = directions[direction];
    }

    //todo: Fix animation on below:::
    if (dead) {
        deadAnimator.animate(&m_texture, direction);
    } else if (powerPelletStateEnd) {
        powerPelletStateEndAnimator.animate(&m_texture, direction);
    } else if (powerPelletState) {
        powerPelletStateAnimator.animate(&m_texture, direction);
    } else {
        m_animator.animate(&m_texture, direction);
    }

    prevDirections = possibleDirectionsVector;
    updateHitbox();

}

Direction Ghost::getDirectionToPlayer(const std::map<Direction, SDL_Rect> &possibleDirections) const {
    Direction closestToPlayer = UP;
    float shortestLength = 1000.0f;
    float longestLength = 0.0f;


    auto playerPosition = Game::getPlayer()->m_positionRectangle;
    for (auto &directionPosition : possibleDirections) {
        int xLen = abs(playerPosition.x - directionPosition.second.x);
        int yLen = abs(playerPosition.y - directionPosition.second.y);
        float lenToPlayer = sqrt((xLen * xLen) + (yLen * yLen));

        if (powerPelletState || switchedToPowerPelletState) {
            if (lenToPlayer > longestLength) {
                longestLength = lenToPlayer;
                closestToPlayer = directionPosition.first;

            }
        } else {
            if (lenToPlayer < shortestLength) {
                shortestLength = lenToPlayer;
                closestToPlayer = directionPosition.first;
            }
        }
    }


    return closestToPlayer;
}

void Ghost::reset() {
    m_positionRectangle.x = 30 * 15;
    m_positionRectangle.y = 30 * 15;
    updateHitbox();
    powerPelletState = false;
}

Uint32 Ghost::powerPelletStateEndCallback(Uint32 n, void *ghost) {
    auto *g = reinterpret_cast<Ghost *>(ghost);
    g->powerPelletStateEnd = true;
    return 0;
}

Uint32 Ghost::reviveGhostCallback(Uint32 n, void *ghost) {
    auto *g = reinterpret_cast<Ghost *>(ghost);
    if(!g->dead){
        g->powerPelletStateEnd = false;
        g->powerPelletState = false;
    }
    return 0;
}





#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

const float GRAVITY = 1200.0f;
const float JUMP_FORCE = -520.0f;
const float MOVE_SPEED = 260.0f;
const float TILE = 40.0f;
const int WIN_W = 800;
const int WIN_H = 600;

struct Rect {
    float left, top, width, height;
    Rect() : left(0), top(0), width(0), height(0) {}
    Rect(float l, float t, float w, float h) : left(l), top(t), width(w), height(h) {}
    float right() const { return left + width; }
    float bottom() const { return top + height; }
    bool intersects(const Rect& o) const {
        return left < o.right() && right() > o.left && top < o.bottom() && bottom() > o.top;
    }
};

enum TileType { EMPTY, GROUND, BRICK, QUESTION, PIPE_TL, PIPE_TR, PIPE_BL, PIPE_BR };
struct Tile { TileType type; bool hasCoin; bool coinUsed; };
struct Coin { Rect r; bool collected; float t; };
struct Goomba { Rect r; float vx; bool alive, squished; float sqTimer, anim; };
struct Particle { sf::Vector2f pos, vel; float life; sf::Color col; };

class Mario {
public:
    Rect r;
    sf::Vector2f vel;
    bool onGround = false, right = true, alive = true;
    float anim = 0, deathTimer = 0;
    int score = 0, coinCount = 0;

    Mario() : r(100, 400, 28, 38) {}

    void update(float dt) {
        anim += dt;
        if (!alive) { deathTimer += dt; vel.y += GRAVITY * dt; r.top += vel.y * dt; return; }
        vel.y += GRAVITY * dt;
        r.left += vel.x * dt;
        r.top += vel.y * dt;
        if (r.top > WIN_H + 120) alive = false;
    }

    void draw(sf::RenderWindow& w, sf::Vector2f cam) {
        sf::Vector2f p(r.left - cam.x, r.top - cam.y);
        if (p.x < -60 || p.x > WIN_W + 60) return;

        auto rs = [&](sf::Vector2f sz, sf::Vector2f pos, sf::Color c) {
            sf::RectangleShape s(sz); s.setPosition(pos); s.setFillColor(c); w.draw(s);
        };
        float W = r.width, H = r.height;

        if (!alive && deathTimer > 0.3f) {
            rs(sf::Vector2f(W, H*0.6f), sf::Vector2f(p.x, p.y+H*0.3f), sf::Color(220,50,50));
            rs(sf::Vector2f(W, H*0.35f), sf::Vector2f(p.x, p.y+H*0.55f), sf::Color(50,50,200));
            rs(sf::Vector2f(W*0.8f, H*0.35f), sf::Vector2f(p.x+W*0.1f, p.y), sf::Color(255,200,150));
            rs(sf::Vector2f(W*0.9f, H*0.15f), sf::Vector2f(p.x+W*0.05f, p.y-H*0.05f), sf::Color(220,50,50));
            float ex1 = p.x+W*0.3f, ex2 = p.x+W*0.6f, ey = p.y+H*0.12f;
            rs(sf::Vector2f(8,2), sf::Vector2f(ex1,ey), sf::Color(20,20,80));
            rs(sf::Vector2f(8,2), sf::Vector2f(ex2,ey), sf::Color(20,20,80));
            rs(sf::Vector2f(2,8), sf::Vector2f(ex1+3,ey-3), sf::Color(20,20,80));
            rs(sf::Vector2f(2,8), sf::Vector2f(ex2+3,ey-3), sf::Color(20,20,80));
            return;
        }

        rs(sf::Vector2f(W*0.95f, H*0.14f), sf::Vector2f(p.x+W*0.02f, p.y), sf::Color(220,30,30));
        rs(sf::Vector2f(W*0.55f, H*0.07f), sf::Vector2f(right ? p.x+W*0.4f : p.x-W*0.05f, p.y+H*0.12f), sf::Color(220,30,30));
        rs(sf::Vector2f(W*0.82f, H*0.32f), sf::Vector2f(p.x+W*0.09f, p.y+H*0.06f), sf::Color(255,210,160));
        rs(sf::Vector2f(3.5f, 4.5f), sf::Vector2f(right ? p.x+W*0.58f : p.x+W*0.2f, p.y+H*0.14f), sf::Color(20,20,80));
        rs(sf::Vector2f(W*0.55f, H*0.055f), sf::Vector2f(p.x+W*0.22f, p.y+H*0.25f), sf::Color(90,40,15));
        rs(sf::Vector2f(W, H*0.3f), sf::Vector2f(p.x, p.y+H*0.35f), sf::Color(220,30,30));
        rs(sf::Vector2f(W, H*0.3f), sf::Vector2f(p.x, p.y+H*0.58f), sf::Color(40,40,210));
        rs(sf::Vector2f(W*0.15f, H*0.28f), sf::Vector2f(p.x+W*0.15f, p.y+H*0.35f), sf::Color(40,40,210));
        rs(sf::Vector2f(W*0.15f, H*0.28f), sf::Vector2f(p.x+W*0.7f, p.y+H*0.35f), sf::Color(40,40,210));
        sf::CircleShape btn(2.5f); btn.setPosition(sf::Vector2f(p.x+W*0.19f, p.y+H*0.48f)); btn.setFillColor(sf::Color(255,215,0)); w.draw(btn);
        sf::CircleShape btn2(2.5f); btn2.setPosition(sf::Vector2f(p.x+W*0.74f, p.y+H*0.48f)); btn2.setFillColor(sf::Color(255,215,0)); w.draw(btn2);
        float wo = onGround ? std::sin(anim*12)*3 : 0;
        rs(sf::Vector2f(W*0.45f, H*0.12f), sf::Vector2f(p.x-1, p.y+H*0.87f+wo), sf::Color(150,70,20));
        rs(sf::Vector2f(W*0.45f, H*0.12f), sf::Vector2f(p.x+W*0.55f, p.y+H*0.87f-wo), sf::Color(150,70,20));
    }
};

class Game {
public:
    Game() {
        win.create(sf::VideoMode(sf::Vector2u(WIN_W, WIN_H)), "Super Mini Mario");
        win.setFramerateLimit(60);
        const char* paths[] = {"arial.ttf","Arial.ttf","/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                               "/System/Library/Fonts/Helvetica.ttc","C:\\Windows\\Fonts\\arial.ttf",nullptr};
        for (int i=0; paths[i]; i++) if (font.openFromFile(paths[i])) break;
        reset();
    }

    void reset() {
        mario = Mario();
        tiles.clear(); coins.clear(); goombas.clear(); particles.clear();
        totalTime = 0; cam = sf::Vector2f(0,0); state = PLAYING;
        buildLevel();
    }

    void buildLevel() {
        LW = 130; LH = 15;
        tiles.assign(LW*LH, {EMPTY,false,false});
        for (int x=0; x<LW; x++) { st(x,LH-1,GROUND); st(x,LH-2,GROUND); }
        for (int x=31; x<=33; x++) { st(x,LH-1,EMPTY); st(x,LH-2,EMPTY); }
        for (int x=62; x<=64; x++) { st(x,LH-1,EMPTY); st(x,LH-2,EMPTY); }
        for (int x=88; x<=90; x++) { st(x,LH-1,EMPTY); st(x,LH-2,EMPTY); }
        setQ(12,9); setQ(16,9); setQ(14,5); setQ(20,9);
        setB(22,9); setQ(23,9); setB(24,9);
        setQ(50,9); setB(51,9); setQ(52,9);
        setQ(70,7); setB(71,7); setQ(72,7);
        setQ(100,9); setQ(102,7);
        for (int x=38;x<=42;x++) setB(x,9);
        for (int x=55;x<=59;x++) setB(x,7);
        for (int x=75;x<=78;x++) setB(x,9);
        for (int x=80;x<=83;x++) setB(x,7);
        for (int x=95;x<=98;x++) setB(x,7);
        for (int x=44;x<=46;x++) setB(x,4);
        setQ(45,3);
        for (int s=0;s<5;s++) for (int h=0;h<=s;h++) st(108+s, LH-3-h, GROUND);
        for (int s=0;s<5;s++) for (int h=0;h<=(4-s);h++) st(115+s, LH-3-h, GROUND);
        addPipe(8, LH-3); addPipe(25, LH-4); addPipe(45, LH-3); addPipe(85, LH-4);
        addCoin(13,7); addCoin(14,7); addCoin(15,7);
        addCoin(39,7); addCoin(40,7); addCoin(41,7);
        addCoin(44,2); addCoin(45,2); addCoin(46,2);
        addCoin(56,5); addCoin(57,5); addCoin(58,5);
        addCoin(34,11); addCoin(35,11);
        addCoin(65,11); addCoin(66,11);
        addCoin(76,7); addCoin(77,7);
        addCoin(96,5); addCoin(97,5);
        addCoin(109,8); addCoin(110,7); addCoin(111,6);
        addGoomba(18,LH-3); addGoomba(28,LH-3); addGoomba(35,LH-3);
        addGoomba(48,LH-3); addGoomba(58,LH-3); addGoomba(73,LH-3);
        addGoomba(83,LH-3); addGoomba(93,LH-3); addGoomba(104,LH-3);
        flagX = 118*TILE; flagY = (LH-3)*TILE;
    }

    void st(int x,int y,TileType t) { if(x>=0&&x<LW&&y>=0&&y<LH) tiles[x+y*LW]={t,false,false}; }
    void setQ(int x,int y) { if(x>=0&&x<LW&&y>=0&&y<LH) tiles[x+y*LW]={QUESTION,true,false}; }
    void setB(int x,int y) { st(x,y,BRICK); }
    void addPipe(int tx,int ty) { st(tx,ty,PIPE_TL); st(tx+1,ty,PIPE_TR); st(tx,ty+1,PIPE_BL); st(tx+1,ty+1,PIPE_BR); }
    void addCoin(int tx,int ty) { coins.push_back({Rect(tx*TILE+10,ty*TILE+5,20,30),false,0}); }
    void addGoomba(int tx,int ty) { goombas.push_back({Rect(tx*TILE+5,ty*TILE,30,34),-70,true,false,0,0}); }

    TileType gt(int x,int y) { return (x<0||x>=LW||y<0||y>=LH)?EMPTY:tiles[x+y*LW].type; }
    bool solid(TileType t) { return t!=EMPTY; }

    void collide(Mario& m) {
        if (m.vel.x > 0) {
            int rx=(int)((m.r.left+m.r.width)/TILE);
            for (int ty=(int)(m.r.top/TILE); ty<=(int)((m.r.top+m.r.height-1)/TILE); ty++)
                if (solid(gt(rx,ty))) { m.r.left=rx*TILE-m.r.width; m.vel.x=0; break; }
        } else if (m.vel.x < 0) {
            int lx=(int)(m.r.left/TILE);
            for (int ty=(int)(m.r.top/TILE); ty<=(int)((m.r.top+m.r.height-1)/TILE); ty++)
                if (solid(gt(lx,ty))) { m.r.left=(lx+1)*TILE; m.vel.x=0; break; }
        }
        m.onGround = false;
        if (m.vel.y > 0) {
            int by=(int)((m.r.top+m.r.height)/TILE);
            for (int tx=(int)(m.r.left/TILE); tx<=(int)((m.r.left+m.r.width-1)/TILE); tx++)
                if (solid(gt(tx,by))) { m.r.top=by*TILE-m.r.height; m.vel.y=0; m.onGround=true; break; }
        } else if (m.vel.y < 0) {
            int ty2=(int)(m.r.top/TILE);
            for (int tx=(int)(m.r.left/TILE); tx<=(int)((m.r.left+m.r.width-1)/TILE); tx++) {
                TileType tt=gt(tx,ty2);
                if (solid(tt)) {
                    m.r.top=(ty2+1)*TILE; m.vel.y=0;
                    int idx=tx+ty2*LW;
                    if (tiles[idx].type==QUESTION && tiles[idx].hasCoin && !tiles[idx].coinUsed) {
                        tiles[idx].coinUsed=true; m.score+=200; m.coinCount++;
                        spawnParticles(tx*TILE+20, ty2*TILE, sf::Color(255,215,0), 6);
                    } else if (tiles[idx].type==BRICK) {
                        tiles[idx].type=EMPTY; m.score+=50;
                        spawnParticles(tx*TILE+20, ty2*TILE+20, sf::Color(180,100,50), 8);
                    }
                    break;
                }
            }
        }
    }

    void updateGoombas(float dt) {
        for (auto& g : goombas) {
            if (!g.alive) continue;
            if (g.squished) { g.sqTimer-=dt; if(g.sqTimer<=0) g.alive=false; continue; }
            g.anim+=dt; g.r.left+=g.vx*dt;
            if (g.vx>0) {
                int rx=(int)((g.r.left+g.r.width)/TILE), my=(int)((g.r.top+g.r.height*0.5)/TILE);
                if (solid(gt(rx,my))) { g.vx=-g.vx; g.r.left=rx*TILE-g.r.width; }
            } else {
                int lx=(int)(g.r.left/TILE), my=(int)((g.r.top+g.r.height*0.5)/TILE);
                if (solid(gt(lx,my))) { g.vx=-g.vx; g.r.left=(lx+1)*TILE; }
            }
            int by=(int)((g.r.top+g.r.height)/TILE);
            bool on=false;
            for (int tx=(int)(g.r.left/TILE); tx<=(int)((g.r.left+g.r.width-1)/TILE); tx++)
                if (solid(gt(tx,by))) { g.r.top=by*TILE-g.r.height; on=true; break; }
            if (!on) g.r.top+=GRAVITY*dt*dt*30;
            if (g.r.top>WIN_H+100) g.alive=false;
        }
    }

    void marioEnemy(Mario& m) {
        for (auto& g : goombas) {
            if (!g.alive||g.squished) continue;
            if (m.r.intersects(g.r)) {
                if (m.vel.y>0 && m.r.top+m.r.height-g.r.top<16) {
                    g.squished=true; g.sqTimer=0.3f;
                    m.vel.y=JUMP_FORCE*0.55f; m.score+=100;
                    spawnParticles(g.r.left+15,g.r.top,sf::Color(160,100,50),4);
                } else { m.alive=false; }
            }
        }
    }

    void collectCoins(Mario& m) {
        for (auto& c : coins) {
            if (c.collected) continue;
            c.t+=0.016f;
            if (m.r.intersects(c.r)) {
                c.collected=true; m.score+=100; m.coinCount++;
                spawnParticles(c.r.left+10,c.r.top+15,sf::Color(255,215,0),8);
            }
        }
    }

    void checkFlag(Mario& m) {
        if (m.r.left+m.r.width>flagX && m.r.left<flagX+TILE && m.r.top+m.r.height>flagY)
            { state=WIN; m.score+=1000; }
    }

    void spawnParticles(float x,float y,sf::Color col,int n) {
        for (int i=0;i<n;i++) {
            float a=((float)i/n)*6.2832f;
            particles.push_back({{x,y},{std::cos(a)*130, std::sin(a)*130-60},0.55f,col});
        }
    }

    void updateParticles(float dt) {
        for (auto& p : particles) { p.pos+=p.vel*dt; p.vel.y+=420*dt; p.life-=dt; }
        particles.erase(std::remove_if(particles.begin(),particles.end(),
            [](const Particle&p){return p.life<=0;}),particles.end());
    }

    void drawBG() {
        win.clear(sf::Color(110,185,235));
        for (int i=0;i<18;i++) {
            float cx = (i*320+80) - cam.x*0.3f;
            float total = LW*TILE*0.3f;
            cx = std::fmod(cx+total+WIN_W, total+WIN_W+200)-200;
            float cy = 30+(i%5)*45;
            sf::CircleShape c1(28); c1.setPosition(sf::Vector2f(cx,cy)); c1.setFillColor(sf::Color(255,255,255,210)); win.draw(c1);
            sf::CircleShape c2(34); c2.setPosition(sf::Vector2f(cx+28,cy-10)); c2.setFillColor(sf::Color(255,255,255,210)); win.draw(c2);
            sf::CircleShape c3(24); c3.setPosition(sf::Vector2f(cx+55,cy)); c3.setFillColor(sf::Color(255,255,255,210)); win.draw(c3);
        }
        for (int i=0;i<22;i++) {
            float hx = (i*380)-cam.x*0.5f;
            float total = LW*TILE*0.5f;
            hx = std::fmod(hx+total+WIN_W, total+WIN_W+200)-200;
            float hy = WIN_H-80-(i%3)*35;
            float rad = 55+(i%3)*35;
            sf::CircleShape hill(rad); hill.setPosition(sf::Vector2f(hx-rad, hy)); hill.setFillColor(sf::Color(85,175,70,170)); win.draw(hill);
        }
    }

    void drawTileAt(TileType t, int tx, int ty) {
        sf::Vector2f p(tx*TILE-cam.x, ty*TILE-cam.y);
        if (p.x<-TILE||p.x>WIN_W+TILE) return;
        auto rs = [&](sf::Vector2f sz, sf::Vector2f pos, sf::Color c) {
            sf::RectangleShape s(sz); s.setPosition(pos); s.setFillColor(c); win.draw(s);
        };
        auto rs2 = [&](sf::Vector2f sz, sf::Vector2f pos, sf::Color c, float ot, sf::Color oc) {
            sf::RectangleShape s(sz); s.setPosition(pos); s.setFillColor(c); s.setOutlineThickness(ot); s.setOutlineColor(oc); win.draw(s);
        };
        switch(t) {
        case GROUND:
            rs(sf::Vector2f(TILE,TILE), p, ty==LH-1 ? sf::Color(139,90,43) : sf::Color(185,125,65));
            rs(sf::Vector2f(TILE,1.5f), sf::Vector2f(p.x,p.y+TILE/2), sf::Color(160,100,40));
            rs(sf::Vector2f(1,TILE), p, sf::Color(120,75,35,70));
            break;
        case BRICK: {
            sf::Color dark(150,80,35);
            rs(sf::Vector2f(TILE,TILE), p, sf::Color(185,105,55));
            for (int i=1;i<3;i++) rs(sf::Vector2f(TILE,1.5f), sf::Vector2f(p.x,p.y+i*TILE/3), dark);
            rs(sf::Vector2f(1.5f,TILE/3), sf::Vector2f(p.x+TILE/2,p.y), dark);
            rs(sf::Vector2f(1.5f,TILE/3), sf::Vector2f(p.x+TILE/4,p.y+TILE/3), dark);
            rs(sf::Vector2f(1.5f,TILE/3), sf::Vector2f(p.x+3*TILE/4,p.y+TILE/3), dark);
            rs2(sf::Vector2f(TILE,TILE), p, sf::Color::Transparent, 1, sf::Color(120,60,20));
            break; }
        case QUESTION: {
            int idx=tx+ty*LW; bool used=tiles[idx].coinUsed;
            float pulse = std::sin(totalTime*4)*18;
            rs(sf::Vector2f(TILE,TILE), p, used ? sf::Color(140,100,60) : sf::Color(255,200+std::min(55.f,pulse),50+std::min(30.f,pulse)));
            rs2(sf::Vector2f(TILE,TILE), p, sf::Color::Transparent, 2, sf::Color(200,150,30));
            if (!used) {
                sf::Color qd(170,110,15);
                rs(sf::Vector2f(14,4), sf::Vector2f(p.x+13,p.y+8), qd);
                rs(sf::Vector2f(4,12), sf::Vector2f(p.x+23,p.y+8), qd);
                rs(sf::Vector2f(14,4), sf::Vector2f(p.x+13,p.y+18), qd);
                rs(sf::Vector2f(4,8), sf::Vector2f(p.x+18,p.y+18), qd);
                rs(sf::Vector2f(4,4), sf::Vector2f(p.x+18,p.y+28), qd);
            }
            break; }
        case PIPE_TL: case PIPE_TR: case PIPE_BL: case PIPE_BR: {
            sf::Color pg(45,175,45), pd(28,135,28), ph(75,215,75);
            rs(sf::Vector2f(TILE,TILE), p, pg);
            float ew = (t==PIPE_TL||t==PIPE_BL) ? 7 : 0;
            float hh = (t==PIPE_TR||t==PIPE_BR) ? 7 : 0;
            if (ew) rs(sf::Vector2f(ew,TILE), p, pd);
            if (hh) rs(sf::Vector2f(hh,TILE), sf::Vector2f(p.x+TILE-hh,p.y), ph);
            if (t==PIPE_TL||t==PIPE_TR) rs(sf::Vector2f(TILE+6,10), sf::Vector2f(p.x-3,p.y), sf::Color(55,195,55));
            break; }
        default: break;
        }
    }

    void drawCoinObj(Coin& c) {
        if (c.collected) return;
        sf::Vector2f p(c.r.left-cam.x, c.r.top-cam.y);
        if (p.x<-TILE||p.x>WIN_W+TILE) return;
        float w = std::abs(std::cos(c.t*4))*c.r.width;
        float xo = (c.r.width-w)/2;
        sf::RectangleShape cs(sf::Vector2f(w,c.r.height));
        cs.setPosition(sf::Vector2f(p.x+xo,p.y)); cs.setFillColor(sf::Color(255,215,0)); win.draw(cs);
        sf::RectangleShape ci(sf::Vector2f(w*0.55f,c.r.height*0.55f));
        ci.setPosition(sf::Vector2f(p.x+xo+w*0.22f,p.y+c.r.height*0.22f)); ci.setFillColor(sf::Color(255,240,100)); win.draw(ci);
    }

    void drawGoombaObj(Goomba& g) {
        if (!g.alive) return;
        sf::Vector2f p(g.r.left-cam.x, g.r.top-cam.y);
        if (p.x<-TILE||p.x>WIN_W+TILE) return;
        auto rs = [&](sf::Vector2f sz, sf::Vector2f pos, sf::Color c) {
            sf::RectangleShape s(sz); s.setPosition(pos); s.setFillColor(c); win.draw(s);
        };
        if (g.squished) {
            rs(sf::Vector2f(g.r.width,10), sf::Vector2f(p.x,p.y+g.r.height-10), sf::Color(160,100,50)); return;
        }
        rs(sf::Vector2f(g.r.width,g.r.height*0.5f), p, sf::Color(185,120,60));
        rs(sf::Vector2f(g.r.width,g.r.height*0.7f), sf::Vector2f(p.x,p.y+g.r.height*0.3f), sf::Color(160,100,50));
        for (int i=0;i<2;i++) {
            rs(sf::Vector2f(9,9), sf::Vector2f(p.x+4+i*(g.r.width-13),p.y+g.r.height*0.14), sf::Color(255,255,255));
            rs(sf::Vector2f(4.5f,4.5f), sf::Vector2f(p.x+6+i*(g.r.width-13),p.y+g.r.height*0.2), sf::Color(10,10,10));
            sf::RectangleShape br(sf::Vector2f(11,3));
            br.setPosition(sf::Vector2f(p.x+2+i*(g.r.width-14),p.y+g.r.height*0.09));
            br.setRotation(sf::degrees(i==0 ? -12.0f : 12.0f));
            br.setFillColor(sf::Color(80,40,15)); win.draw(br);
        }
        float wa = std::sin(g.anim*7)*2.5f;
        rs(sf::Vector2f(13,9), sf::Vector2f(p.x-2,p.y+g.r.height-9+wa), sf::Color(55,28,8));
        rs(sf::Vector2f(13,9), sf::Vector2f(p.x+g.r.width-11,p.y+g.r.height-9-wa), sf::Color(55,28,8));
    }

    void drawFlag() {
        float fx=flagX-cam.x, fy=flagY-cam.y;
        if (fx<-80||fx>WIN_W+80) return;
        sf::RectangleShape pole(sf::Vector2f(5,WIN_H-fy));
        pole.setPosition(sf::Vector2f(fx+17,fy)); pole.setFillColor(sf::Color(110,110,110)); win.draw(pole);
        sf::ConvexShape flag; flag.setPointCount(3);
        flag.setPoint(0,sf::Vector2f(fx+22,fy+2));
        flag.setPoint(1,sf::Vector2f(fx+65,fy+22));
        flag.setPoint(2,sf::Vector2f(fx+22,fy+42));
        flag.setFillColor(sf::Color(10,190,10)); win.draw(flag);
        sf::CircleShape star(5); star.setPosition(sf::Vector2f(fx+36,fy+16)); star.setFillColor(sf::Color(255,255,100)); win.draw(star);
        sf::CircleShape ball(7); ball.setPosition(sf::Vector2f(fx+16,fy-7)); ball.setFillColor(sf::Color(255,215,0)); win.draw(ball);
    }

    void drawHUD() {
        auto txt = [&](const std::string& s, sf::Color c, float x, float y, unsigned int sz=20) {
            sf::Text t(font, s, sz);
            t.setFillColor(c); t.setOutlineColor(sf::Color(0,0,0)); t.setOutlineThickness(2);
            t.setPosition(sf::Vector2f(x,y)); win.draw(t);
        };
        txt("SCORE: "+std::to_string(mario.score), sf::Color::White, 10, 8);
        txt("COINS: "+std::to_string(mario.coinCount), sf::Color(255,215,0), 10, 32);
        txt("Arrows/WASD: Move | Space/Up: Jump | R: Restart", sf::Color(255,255,255,180), (float)(WIN_W/2-195), (float)(WIN_H-24), 13);
    }

    void drawEnd(bool isWin) {
        sf::RectangleShape ov(sf::Vector2f(WIN_W,WIN_H));
        ov.setFillColor(sf::Color(0,0,0,160)); win.draw(ov);
        sf::Text t(font, isWin?"YOU WIN!":"GAME OVER", 52);
        t.setFillColor(isWin?sf::Color(255,215,0):sf::Color(255,50,50));
        t.setOutlineColor(sf::Color::Black); t.setOutlineThickness(4);
        t.setPosition(sf::Vector2f(WIN_W/2-t.getGlobalBounds().size.x/2, WIN_H/2-70)); win.draw(t);
        sf::Text s(font, "FINAL SCORE: "+std::to_string(mario.score), 26);
        s.setFillColor(sf::Color::White); s.setOutlineColor(sf::Color::Black); s.setOutlineThickness(2);
        s.setPosition(sf::Vector2f(WIN_W/2-s.getGlobalBounds().size.x/2, WIN_H/2+5)); win.draw(s);
        sf::Text r(font, "Press R to Restart", 20);
        r.setFillColor(sf::Color(200,200,200)); r.setOutlineColor(sf::Color::Black); r.setOutlineThickness(1);
        r.setPosition(sf::Vector2f(WIN_W/2-r.getGlobalBounds().size.x/2, WIN_H/2+50)); win.draw(r);
    }

    void run() {
        sf::Clock clk;
        while (win.isOpen()) {
            float dt = clk.restart().asSeconds();
            if (dt>0.05f) dt=0.05f;
            totalTime += dt;

            while (auto e = win.pollEvent()) {
                if (e->is<sf::Event::Closed>()) win.close();
                if (auto* kp = e->getIf<sf::Event::KeyPressed>()) {
                    if (kp->code == sf::Keyboard::Key::R) reset();
                    if (kp->code == sf::Keyboard::Key::Escape) win.close();
                }
            }

            if (state==PLAYING && mario.alive) {
                mario.vel.x = 0;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)||sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
                    { mario.vel.x=-MOVE_SPEED; mario.right=false; }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)||sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
                    { mario.vel.x=MOVE_SPEED; mario.right=true; }
                if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)||sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)||sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) && mario.onGround)
                    { mario.vel.y=JUMP_FORCE; mario.onGround=false; }

                mario.update(dt);
                collide(mario);
                updateGoombas(dt);
                marioEnemy(mario);
                collectCoins(mario);
                checkFlag(mario);
                updateParticles(dt);

                float target = mario.r.left - WIN_W/3;
                cam.x += (target-cam.x)*6*dt;
                cam.x = std::max(0.f, std::min(cam.x, LW*TILE-WIN_W));
                if (!mario.alive) state=GAME_OVER;
            } else {
                updateParticles(dt);
                if (!mario.alive) mario.update(dt);
            }

            drawBG();
            int sx=std::max(0,(int)(cam.x/TILE)-1), ex=std::min(LW,(int)((cam.x+WIN_W)/TILE)+2);
            for (int ty=0;ty<LH;ty++) for (int tx=sx;tx<ex;tx++)
                if (tiles[tx+ty*LW].type!=EMPTY) drawTileAt(tiles[tx+ty*LW].type, tx, ty);
            for (auto& c : coins) drawCoinObj(c);
            for (auto& g : goombas) drawGoombaObj(g);
            drawFlag();
            mario.draw(win, cam);
            for (auto& p : particles) {
                float a = std::min(1.f, p.life/0.3f);
                sf::CircleShape dot(3.5f);
                dot.setPosition(sf::Vector2f(p.pos.x-cam.x, p.pos.y-cam.y));
                p.col.a = (uint8_t)(a*255); dot.setFillColor(p.col); win.draw(dot);
            }
            drawHUD();
            if (state==GAME_OVER) drawEnd(false);
            if (state==WIN) drawEnd(true);
            win.display();
        }
    }

private:
    sf::RenderWindow win;
    sf::Font font;
    Mario mario;
    std::vector<Tile> tiles;
    std::vector<Coin> coins;
    std::vector<Goomba> goombas;
    std::vector<Particle> particles;
    sf::Vector2f cam;
    int LW, LH;
    float flagX, flagY, totalTime=0;
    enum { PLAYING, GAME_OVER, WIN } state = PLAYING;
};

int main() { Game g; g.run(); return 0; }
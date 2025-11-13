/* battle.c
   Batalha Pokémon 2 jogadores - versão em C (Geração I, dano apenas)
   Autores: adaptado para exercício de sala
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LEVEL 50
#define MAX_PKM 3
#define MAX_MOVES 4

// tipos simplificados para cálculo de eficácia
typedef enum {T_FIRE, T_WATER, T_GRASS, T_NORMAL} Type;

// estrutura de movimento
typedef struct {
    char name[32];
    Type type;
    int power; // Power do movimento (Gen I)
} Move;

// estrutura de pokemon (valores base e stats calculados)
typedef struct {
    char name[32];
    Type type; // só 1 tipo para simplificar
    int base_hp;
    int base_atk;
    int base_def;
    int base_spc; // special (Geração I)
    int base_spe;

    int hp;   // HP atual
    int max_hp;
    int atk;
    int def;
    int spc;
    int spe;

    Move moves[MAX_MOVES];
    int alive;
} Pokemon;

// função para calcular stats (fórmulas padrão, IV=0, EV=0 simplificadas)
int calc_stat(int base) {
    // stat = ((base * 2) * LEVEL) / 100 + 5
    return ((base * 2) * LEVEL) / 100 + 5;
}
int calc_hp(int base) {
    // HP = ((base * 2) * LEVEL) / 100 + LEVEL + 10
    return ((base * 2) * LEVEL) / 100 + LEVEL + 10;
}

// multiplicador de eficácia simples: 2.0 se super efetivo, 0.5 se pouco efetivo, 1 caso contrário
double type_effectiveness(Type moveType, Type targetType) {
    if (moveType == T_FIRE && targetType == T_GRASS) return 2.0;
    if (moveType == T_FIRE && targetType == T_WATER) return 0.5;
    if (moveType == T_WATER && targetType == T_FIRE) return 2.0;
    if (moveType == T_WATER && targetType == T_GRASS) return 0.5;
    if (moveType == T_GRASS && targetType == T_WATER) return 2.0;
    if (moveType == T_GRASS && targetType == T_FIRE) return 0.5;
    return 1.0;
}

// STAB: 1.5 se mesmo tipo, 1.0 senão (Geração I usa 1.5)
double stab(Type moveType, Type userType) {
    return (moveType == userType) ? 1.5 : 1.0;
}

// fórmula de dano (Geração I simplificada, sem crítico):
// damage = (((((2*Level)/5)+2) * Power * A/D)/50 + 2) * modifier
// onde modifier = STAB * type_effectiveness * (random 217..255 / 255)
// usamos inteiros no núcleo e multiplicamos por um double no final.
int calculate_damage(Pokemon *attacker, Pokemon *defender, Move *move) {
    int A; // usa Attack para movimentos físicos, Special para ataques especiais
    int D;
    // Em Gen I, categoria move: vamos considerar Fire/Water/Grass moves como "special" e Tackle como physical
    int physical = (strcmp(move->name, "Tackle") == 0) ? 1 : 0;
    if (physical) {
        A = attacker->atk;
        D = defender->def;
    } else {
        A = attacker->spc;
        D = defender->spc;
    }

    int term1 = (2 * LEVEL) / 5 + 2;
    int base = (term1 * move->power * A) / D;
    base = base / 50 + 2;

    // multiplicadores
    double m_stab = stab(move->type, attacker->type);
    double m_type = type_effectiveness(move->type, defender->type);
    int r = (rand() % (255 - 217 + 1)) + 217; // 217..255
    double m_rand = (double)r / 255.0;

    double modifier = m_stab * m_type * m_rand;
    int damage = (int)(base * modifier);

    if (damage < 1) damage = 1;
    return damage;
}

// mostre opções de moves
void print_moves(Pokemon *p) {
    for (int i = 0; i < MAX_MOVES; ++i) {
        printf("%d) %s (Power %d)\n", i+1, p->moves[i].name, p->moves[i].power);
    }
    printf("5) Trocar de Pokémon\n");
}

// imprime resumo do pokemon
void print_pkm_status(Pokemon *p) {
    printf("%s - HP: %d/%d\n", p->name, p->hp, p->max_hp);
}

// inicializa pokemons (usando base stats da Bulbapedia para Geração I)
void init_pokemon(Pokemon *p, const char *name, Type type, int bhp, int batk, int bdef, int bspc, int bspd) {
    strcpy(p->name, name);
    p->type = type;
    p->base_hp = bhp;
    p->base_atk = batk;
    p->base_def = bdef;
    p->base_spc = bspc;
    p->base_spe = bspd;

    p->max_hp = calc_hp(bhp);
    p->hp = p->max_hp;
    p->atk = calc_stat(batk);
    p->def = calc_stat(bdef);
    p->spc = calc_stat(bspc);
    p->spe = calc_stat(bspd);
    p->alive = 1;
}

// cria times fixos (cada jogador tem os mesmos 3 pokemons para simplicidade)
void create_teams(Pokemon team1[], Pokemon team2[]) {
    // Base stats (Geração I) Fonte: Bulbapedia lista de base stats
    // Charizard: base_hp 78, atk 84, def 78, spc 85, spe 100
    init_pokemon(&team1[0], "Charizard", T_FIRE, 78, 84, 78, 85, 100);
    init_pokemon(&team1[1], "Blastoise", T_WATER, 79, 83, 100, 78, 85);
    init_pokemon(&team1[2], "Venusaur", T_GRASS, 80, 82, 83, 100, 80);
    // mesmas cópias para o jogador 2
    init_pokemon(&team2[0], "Charizard", T_FIRE, 78, 84, 78, 85, 100);
    init_pokemon(&team2[1], "Blastoise", T_WATER, 79, 83, 100, 78, 85);
    init_pokemon(&team2[2], "Venusaur", T_GRASS, 80, 82, 83, 100, 80);

    // moves: cada pokemon tem 2 especiais e 2 tackles por exemplo
    // Charizard
    strcpy(team1[0].moves[0].name, "Flamethrower");
    team1[0].moves[0].type = T_FIRE; team1[0].moves[0].power = 95;
    strcpy(team1[0].moves[1].name, "Fire Punch"); // filler (power 75)
    team1[0].moves[1].type = T_FIRE; team1[0].moves[1].power = 75;
    strcpy(team1[0].moves[2].name, "Tackle"); team1[0].moves[2].type = T_NORMAL; team1[0].moves[2].power = 35;
    strcpy(team1[0].moves[3].name, "Heat Wave"); team1[0].moves[3].type = T_FIRE; team1[0].moves[3].power = 95;

    // Blastoise
    strcpy(team1[1].moves[0].name, "Hydro Pump");
    team1[1].moves[0].type = T_WATER; team1[1].moves[0].power = 120;
    strcpy(team1[1].moves[1].name, "Surf"); team1[1].moves[1].type = T_WATER; team1[1].moves[1].power = 90;
    strcpy(team1[1].moves[2].name, "Tackle"); team1[1].moves[2].type = T_NORMAL; team1[1].moves[2].power = 35;
    strcpy(team1[1].moves[3].name, "Bite"); team1[1].moves[3].type = T_NORMAL; team1[1].moves[3].power = 60;

    // Venusaur
    strcpy(team1[2].moves[0].name, "Solar Beam");
    team1[2].moves[0].type = T_GRASS; team1[2].moves[0].power = 120;
    strcpy(team1[2].moves[1].name, "Razor Leaf"); team1[2].moves[1].type = T_GRASS; team1[2].moves[1].power = 55;
    strcpy(team1[2].moves[2].name, "Tackle"); team1[2].moves[2].type = T_NORMAL; team1[2].moves[2].power = 35;
    strcpy(team1[2].moves[3].name, "Sludge"); team1[2].moves[3].type = T_NORMAL; team1[2].moves[3].power = 65;

    // copiar moves para o time 2
    for (int i = 0; i < MAX_PKM; ++i) {
        for (int j = 0; j < MAX_MOVES; ++j) {
            team2[i].moves[j] = team1[i].moves[j];
        }
    }
}

// escolhe primeiro pokemon ativo para cada jogador
int choose_starting_pokemon(Pokemon team[], int player) {
    printf("Jogador %d, escolha seu pokemon inicial (1-%d):\n", player, MAX_PKM);
    for (int i = 0; i < MAX_PKM; ++i) {
        printf("%d) %s (HP %d)\n", i+1, team[i].name, team[i].max_hp);
    }
    int opt;
    do {
        scanf("%d", &opt);
    } while (opt < 1 || opt > MAX_PKM);
    return opt - 1;
}

// verifica se time tem pokemons vivos
int any_alive(Pokemon team[]) {
    for (int i = 0; i < MAX_PKM; ++i) if (team[i].alive) return 1;
    return 0;
}

int main() {
    srand((unsigned)time(NULL));
    Pokemon team1[MAX_PKM], team2[MAX_PKM];
    create_teams(team1, team2);

    printf("=== Batalha Pokémon (2 jogadores) - Nível %d ===\n", LEVEL);
    int active1 = choose_starting_pokemon(team1, 1);
    int active2 = choose_starting_pokemon(team2, 2);

    while (any_alive(team1) && any_alive(team2)) {
        // mostra status
        printf("\n--- Status ---\n");
        printf("Jogador 1 ativo: "); print_pkm_status(&team1[active1]);
        printf("Jogador 2 ativo: "); print_pkm_status(&team2[active2]);
        printf("----------------\n");

        // determina ordem por speed (se empate, jogador 1 primeiro)
        int firstPlayer = 1;
        if (team2[active2].spe > team1[active1].spe) firstPlayer = 2;

        for (int turnPlayer = 0; turnPlayer < 2; ++turnPlayer) {
            int current = (turnPlayer == 0) ? firstPlayer : (firstPlayer == 1 ? 2 : 1);

            if (current == 1) {
                if (!team1[active1].alive) continue;
                printf("\nVez do Jogador 1 (Pokemon: %s)\n", team1[active1].name);
                print_moves(&team1[active1]);
                int choice;
                do { scanf("%d", &choice); } while (choice < 1 || choice > 5);
                if (choice == 5) {
                    // trocar
                    printf("Escolha qual trocar (1-%d):\n", MAX_PKM);
                    for (int i=0;i<MAX_PKM;++i) {
                        printf("%d) %s %s\n", i+1, team1[i].name, team1[i].alive ? "" : "(KO)");
                    }
                    int t;
                    do { scanf("%d", &t); } while (t < 1 || t > MAX_PKM || !team1[t-1].alive);
                    active1 = t-1;
                    printf("Jogador 1 trocou para %s\n", team1[active1].name);
                } else {
                    int dmg = calculate_damage(&team1[active1], &team2[active2], &team1[active1].moves[choice-1]);
                    team2[active2].hp -= dmg;
                    if (team2[active2].hp <= 0) { team2[active2].hp = 0; team2[active2].alive = 0; }
                    printf("%s usou %s e causou %d de dano!\n", team1[active1].name, team1[active1].moves[choice-1].name, dmg);
                    if (!team2[active2].alive) {
                        printf("%s foi derrotado!\n", team2[active2].name);
                        // se houver poke vivo, pedir troca automática
                        if (any_alive(team2)) {
                            printf("Jogador 2, escolha novo pokemon (1-%d):\n", MAX_PKM);
                            for (int i=0;i<MAX_PKM;++i) {
                                if (team2[i].alive) printf("%d) %s (HP %d)\n", i+1, team2[i].name, team2[i].hp);
                            }
                            int t; do { scanf("%d", &t); } while (t < 1 || t > MAX_PKM || !team2[t-1].alive);
                            active2 = t-1;
                        } else {
                            break;
                        }
                    }
                }
            } else { // jogador 2
                if (!team2[active2].alive) continue;
                printf("\nVez do Jogador 2 (Pokemon: %s)\n", team2[active2].name);
                print_moves(&team2[active2]);
                int choice;
                do { scanf("%d", &choice); } while (choice < 1 || choice > 5);
                if (choice == 5) {
                    printf("Escolha qual trocar (1-%d):\n", MAX_PKM);
                    for (int i=0;i<MAX_PKM;++i) {
                        printf("%d) %s %s\n", i+1, team2[i].name, team2[i].alive ? "" : "(KO)");
                    }
                    int t;
                    do { scanf("%d", &t); } while (t < 1 || t > MAX_PKM || !team2[t-1].alive);
                    active2 = t-1;
                    printf("Jogador 2 trocou para %s\n", team2[active2].name);
                } else {
                    int dmg = calculate_damage(&team2[active2], &team1[active1], &team2[active2].moves[choice-1]);
                    team1[active1].hp -= dmg;
                    if (team1[active1].hp <= 0) { team1[active1].hp = 0; team1[active1].alive = 0; }
                    printf("%s usou %s e causou %d de dano!\n", team2[active2].name, team2[active2].moves[choice-1].name, dmg);
                    if (!team1[active1].alive) {
                        printf("%s foi derrotado!\n", team1[active1].name);
                        if (any_alive(team1)) {
                            printf("Jogador 1, escolha novo pokemon (1-%d):\n", MAX_PKM);
                            for (int i=0;i<MAX_PKM;++i) {
                                if (team1[i].alive) printf("%d) %s (HP %d)\n", i+1, team1[i].name, team1[i].hp);
                            }
                            int t; do { scanf("%d", &t); } while (t < 1 || t > MAX_PKM || !team1[t-1].alive);
                            active1 = t-1;
                        } else {
                            break;
                        }
                    }
                }
            }

            // se algum time foi derrotado, sair do loop de turnos
            if (!any_alive(team1) || !any_alive(team2)) break;
        } // fim dos dois turnos
    } // fim loop de batalha

    if (any_alive(team1)) printf("\n== Jogador 1 venceu a batalha! ==\n");
    else printf("\n== Jogador 2 venceu a batalha! ==\n");

    return 0;
}

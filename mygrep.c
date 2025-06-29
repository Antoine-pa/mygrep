#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/*regex est un tableau d'entiers de taille length qui représentent des lettres */
struct regex {
    int length;
    int* content;
};
typedef struct regex regex;

/*tree repésente un noeud étiqueté par value qui pointent vers 2 tree (ses enfants)*/
struct tree {
    int value;
    struct tree* left_child;
    struct tree* right_child;
};
typedef struct tree tree;

/*une pile d'arbres pour le parser*/
struct stack {
    int capacity;
    int size;
    tree** data;
};
typedef struct stack stack;

/*une liste chainée pour l'automate et les calculs pour l'algorithme de Berry Sethi*/
struct list {
    struct list* next;
    int data;
};
typedef struct list list;

/*
automate est non déterministe
delta :
- tableau de taille nb_etats
    - chaque case contient un tableau de taille 256 (une case pour chaque caractère ascii)
        - chaque case des sous tableaux contient une liste d'états d'arrivés car l'automate est non déterministe
*/
struct automate {
    int nb_etats;
    int etat_initial;
    bool* etats_finaux;
    list*** delta;
};
typedef struct automate automate;

/*====================regex=====================*/
/*renvoie la longueur d'une string*/
int length_char_array(char* array) {
    int i = 0;
    while(array[i] != '\0') {
        i++;
    }
    return i;
}

/*transforme une chaine de charactère en regex*/
regex* char_array_to_regex(char* array) {
    regex* re = malloc(sizeof(regex));
    re->length = length_char_array(array);
    re->content = malloc(sizeof(int)*re->length);
    for(int i = 0; i<re->length; i++) {
        re->content[i] = (int)array[i];
    }
    return re;
}

void free_regex(regex* re) {
    free(re->content);
    free(re);
}

/*====================tree=====================*/
/*initialise un arbre qui ne contient qu'un noeud*/
tree* init_tree(int value) {
    tree* t = malloc(sizeof(tree));
    t->value = value;
    t->left_child = NULL;
    t->right_child = NULL;
    return t;
}

/*libère les arbres récursivement*/
void free_tree(tree* t) {
    if(t == NULL) {
        return;
    }
    free_tree(t->left_child);
    free_tree(t->right_child);
    free(t);
}

/*====================stack=====================*/
/*initialise une pile vide de capacité 4*/
stack* init_stack() {
    stack* s = malloc(sizeof(stack));
    s->data = malloc(sizeof(tree*)*4);
    s->capacity = 4;
    s->size = 0;
    return s;
}

/*vérifie si la pile est vide*/
bool stack_is_empty(stack* s) {
    return s->size == 0;
}

/*libère une pile sans libérer son contenu*/
void free_stack(stack* s) {
    free(s->data);
    free(s);
}

/*ajoute un élément à la pile et double sa taille si elle est pleine*/
void push(stack* s, tree* value) {
    s->data[s->size] = value;
    s->size++;
    if (s->size == s->capacity) {
        //on double la taille de la pile
        s->capacity = s->capacity*2;
        tree** data = malloc(sizeof(tree*)*s->capacity);
        for(int i = 0; i<s->size;i++) {
            data[i] = s->data[i];
        }
        free(s->data);
        s->data = data;
    }
}

/*dépile un élément et divise la taille de la pile par 2 si elle est trop vide*/
tree* pop(stack* s) {
    if (stack_is_empty(s)) {
        return NULL;
    }
    s->size--;
    tree* value = s->data[s->size];
    s->data[s->size] = NULL;
    if(3 < s->capacity && s->size <= s->capacity/4) {
        s->capacity = s->capacity/2;
        tree** data = malloc(sizeof(tree*)*s->capacity);
        for(int i = 0; i<s->size; i++) {
            data[i] = s->data[i];
        }
        free(s->data);
        s->data = data;
    }
    return value;
}

/*====================list=====================*/
/*renvoie une liste vide*/
list* empty_list() {
    return NULL;
}

/*créé une liste contenant un élément n*/
list* create_list(int n){
    list* l = malloc(sizeof(list));
    l->data = n;
    l->next = NULL;
    return l;
}

/*renvoie une liste contenant un élément en plus en tête (ne modifie pas old)*/
list* list_prepend(list* old, int n){
    list* l = create_list(n);
    l->next = old;
    return l;
}

/*renvoie une liste contenant le premier élément en moins en le libérant de la mémoire*/
list* list_remove_first(list* l){
    list* next = l->next;
    free(l);
    return next;
}

/*libère une liste*/
void free_list(list* l) {
    if(l != NULL) {
        free_list(l->next);
        free(l);
    }
}

/*renvoie l'union de 2 listes dans une nouvelle liste et libère les 2 listes initiales si delete est vrai*/
list* union_list(list* l1, list* l2, bool delete) {
    list* l = empty_list();
    while(l1 != NULL) {
        l = list_prepend(l, l1->data);
        if(delete) {
            l1 = list_remove_first(l1);
        }
        else {
            l1 = l1->next;
        }
    }
    while(l2 != NULL) {
        l = list_prepend(l, l2->data);
        if(delete) {
            l2 = list_remove_first(l2);
        }
        else {
            l2 = l2->next;
        }
    }
    return l;
}

/*renvoie le produit cartésien de 2 listes sous forme d'un tableau de taille 2 et libère les 2 listes initiales si delete est vrai :
{[1, 2], [3, 4]} représente la liste [(1, 3), (2, 4)]*/
list** prod_int_list(list* l1, list* l2, bool delete) {
    list** l = malloc(sizeof(list*)*2);
    l[0] = empty_list();
    l[1] = empty_list();
    list* l2_c = l2;
    while(l1!=NULL) {
        while(l2_c != NULL) {
            l[0] = list_prepend(l[0], l1->data);
            l[1] = list_prepend(l[1], l2_c->data);
            l2_c = l2_c->next;
        }
        l2_c = l2;
        if(delete) {
            l1 = list_remove_first(l1);
        }
        else {
            l1 = l1->next;
        }
    }
    if(delete) {
        free_list(l2);
    }
    return l;
}

/*====================outils de résolution du problème=====================*/
/*renvoie vrai si c représente @, ?, * ou |*/
bool is_symbol(int c) {
    return c == (int)'@' || c == (int)'|' || c == (int)'*' || c == (int)'?';
}

/*renvoie le nombre de paramètre que prend une opération*/
int arite_symbol(int c) {
    if (c == (int)'@' || c == (int)'|') {
        return 2;
    }
    return 1;
}

/*renvoie le nombre de lettres que contient une expression (ε n'en est pas une)*/
int nb_letters(tree* t) {
    if(t == NULL) {
        return 0;
    }
    if(t->left_child == NULL && t->value >=0) {
        return 1;
    }
    return nb_letters(t->right_child) + nb_letters(t->left_child);
}

/*====================linéarisation dans l'arbre=====================*/
/*remplace les expression f? par fε|
ε est représenté par l'entier -1*/
void _replace_interog(tree* t, tree* t_parent) {
    if(t == NULL) {
        return;
    }
    if(t->value == (int)'?') {
        tree* or = init_tree((int)'|');
        or->left_child = t->left_child;
        or->right_child = init_tree(-1);
        if(t_parent->left_child == t) {
            t_parent->left_child = or;
        }
        else {
            t_parent->right_child = or;
        }
        free(t);
        _replace_interog(or->left_child, or);
    }
    else {
        _replace_interog(t->right_child, t);
        _replace_interog(t->left_child, t);
    }
}

tree* replace_interog(tree* t) {
    if(t != NULL && t->value == (int)'?') {
        tree* or = init_tree((int)'|');
        or->left_child = t->left_child;
        or->right_child = init_tree(-1);
        free(t);
        _replace_interog(or->left_child, NULL);
        return or;
    }
    _replace_interog(t, NULL);
    return t;
}

/*remplit un tableau faisant la correspondance entre une lettre et un entier et linéarise l'expression rationnelle dans l'arbre:
tab_corres est de taille nombre de lettre dans l'expression d'origine et à chaque case associe un entier unique
*/
void create_correspondence(tree* t, int** tab_corres, int* i) {
    if(t == NULL) {
        return;
    }
    if(t->left_child == NULL && t->value >=0) {
        (*tab_corres)[*i] = t->value;
        t->value = *i;
        *i = *i+1;
        return;
    }
    create_correspondence(t->left_child, tab_corres, i);
    create_correspondence(t->right_child, tab_corres, i);
}

/*parse créer un arbre représentant l'expression rationnelle linéarisée et un tableau de correspondance entier <=> lettre
Pour cela, on utilise une pile servant à construire l'arbre final. Une fois l'arbré créer, on supprime les symboles ?*/
tree* parse(regex* re, int** tab_corres) {
    stack* expression = init_stack();
    for(int i = 0; i<re->length; i++) {
        if(is_symbol(re->content[i])) {
            tree* t = init_tree(re->content[i]);
            if(arite_symbol(re->content[i]) == 1) {
                t->left_child = pop(expression);
                //t->right_child = NULL; //inutile mais important à savoir
            }
            else {
                t->right_child = pop(expression); //l'ordre de remplissage compte pour la concaténation
                t->left_child = pop(expression);
            }
            push(expression, t);
        }
        else {
            push(expression, init_tree(re->content[i]));
        }
    }
    tree* t = pop(expression);
    free_stack(expression);
    t = replace_interog(t);
    *tab_corres = malloc(sizeof(int)*nb_letters(t));
    int i = 0;
    create_correspondence(t, tab_corres, &i);
    return t;
}

/*====================calcul des ensembles P, D et F=====================*/
/*renvoie vrai si t reconnait epsilon, faux sinon :*/
bool admit_epsilon(tree* t) {
    if(t == NULL || (t->left_child == NULL && t->value >= 0)) {
        return false;
    }
    if(t->value == -1 || t->value == (int)'*') {
        return true;
    }
    if(t->value == (int)'|') {
        return admit_epsilon(t->left_child) || admit_epsilon(t->right_child);
    }
    return admit_epsilon(t->right_child) && admit_epsilon(t->left_child);
}

/*renvoie la liste d'entiers représentant l'ensemble P :*/
list* calc_P(tree* t) {
    if(t == NULL || t->value == -1) {
        return empty_list();
    }
    else if(t->left_child == NULL) {
        return create_list(t->value);
    }
    else if(t->value == (int)'|') {
        return union_list(calc_P(t->left_child), calc_P(t->right_child), true);
    }
    else if(t->value == (int)'@' && admit_epsilon(t->left_child)) {
        return union_list(calc_P(t->left_child), calc_P(t->right_child), true);
    }
    else if(t->value == (int)'@') {
        return calc_P(t->left_child);
    }
    else {
        return calc_P(t->left_child);
    }
}

/*renvoie la liste d'entiers représentant l'ensemble D :*/
list* calc_D(tree* t) {
    if(t == NULL || t->value == -1) {
        return empty_list();
    }
    else if(t->left_child == NULL) {
        return create_list(t->value);
    }
    else if(t->value == (int)'|') {
        return union_list(calc_D(t->left_child), calc_D(t->right_child), true);
    }
    else if(t->value == (int)'@' && admit_epsilon(t->right_child)) {
        return union_list(calc_D(t->left_child), calc_D(t->right_child), true);
    }
    else if(t->value == (int)'@') {
        return calc_D(t->right_child);
    }
    else {
        return calc_D(t->left_child);
    }
}

/*renvoie une liste de couples vide :*/
list** empty_couple_list() {
    list** l = malloc(sizeof(list*)*2);
    l[0] = empty_list();
    l[1] = empty_list();
    return l;
}

/*fait l'union de deux listes de couples :*/
list** union_couple_list(list** l1, list** l2, bool delete) {
    list** l3 = empty_couple_list();
    l3[0] = union_list(l1[0], l2[0], delete);
    l3[1] = union_list(l1[1], l2[1], delete);
    if(delete) {
        free(l1);
        free(l2);
    }
    return l3;
}

/*renvoie la liste de couples d'entiers représentant l'ensemble F :*/
list** calc_F(tree* t) {
    if(t == NULL || t->left_child == NULL) {
        return empty_couple_list();
    }
    else if(t->value == (int)'|') {
        return union_couple_list(calc_F(t->left_child), calc_F(t->right_child), true);
    }
    else if(t->value == (int)'@') {
        return union_couple_list(union_couple_list(calc_F(t->left_child), calc_F(t->right_child), true), prod_int_list(calc_D(t->left_child), calc_P(t->right_child), true), true);
    }
    else {
        return union_couple_list(calc_F(t->left_child), prod_int_list(calc_D(t->left_child), calc_P(t->left_child), true), true);
    }
}

/*====================automates=====================*/
/*permet de représenter l'automate a sous forme de fichier .viz qu'on peut ouvrir sur un site de lecture d'automate :*/
void ecrire_automate(automate* a, char* nom) {
    FILE* flux = fopen(nom, "w");
    fprintf(flux, "digraph a {\nrankdir = LR;\n");
    for (int i = 0; i < a->nb_etats; i++)
    {
        if (a->etats_finaux[i]) 
        {
            fprintf(flux, "node [shape = doublecircle, label = %d] %d;\n", i, i);
        }
        else
        {
            fprintf(flux, "node [shape = circle, label = %d] %d;\n", i, i);
        }
    }
    fprintf(flux, "node [shape = point]; I\n");
    fprintf(flux, "I -> %d;\n", a->etat_initial);
    for (int i = 0; i < a->nb_etats; i++)
    {
        for (int j = 0; j < 256; j++)
        {   
            list* l = a->delta[i][j];
            while(l != NULL)
            {
                fprintf(flux, "%d -> %d [label = \"%c\"];\n", i, l->data, (char) (j));
                l = l->next;
            }
        }
    }
    fprintf(flux, "}");
    fclose(flux);
}

/*libère l'automate :*/
void free_automate(automate* a) {
    free(a->etats_finaux);
    for(int i = 0; i<a->nb_etats; i++) {
        for(int j = 0; j<256; j++) {
            free_list(a->delta[i][j]);
        }
        free(a->delta[i]);
    }
    free(a->delta);
    free(a);
}

/*construit l'automate reconnaissant le même langage que le langage dénoté par l'expression représentée par t, par Berry Sethi étendu :*/
automate* create_automate(tree* t, int* tab_corres) {
    list* p = calc_P(t);
    list* d = calc_D(t);
    list** f= calc_F(t);
    list* parc_p = p;
    list* parc_d = d;
    list* parc_f0 = f[0];
    list* parc_f1 = f[1];
    automate* a = malloc(sizeof(automate));
    a->nb_etats = nb_letters(t)+1;
    a->etat_initial = 0;
    a->etats_finaux = malloc(sizeof(bool)*a->nb_etats);
    for(int i = 0; i<a->nb_etats; i++) {
        a->etats_finaux[i] = false;
    }
    while(parc_d != NULL) {
        a->etats_finaux[parc_d->data+1] = true;
        parc_d = parc_d->next;
    }
    a->etats_finaux[0] = admit_epsilon(t);
    a->delta = malloc(sizeof(list**)*a->nb_etats);
    for(int i = 0; i<a->nb_etats; i++) {
        a->delta[i] = malloc(sizeof(list*)*256);
        for(int j = 0; j<256; j++) {
            a->delta[i][j] = empty_list();
        }
    }
    while(parc_p != NULL) {
        a->delta[0][tab_corres[parc_p->data]] = list_prepend(a->delta[0][tab_corres[parc_p->data]], parc_p->data+1);
        parc_p = parc_p->next;
    }
    while(parc_f0 != NULL) {
        a->delta[parc_f0->data+1][tab_corres[parc_f1->data]] = list_prepend(a->delta[parc_f0->data+1][tab_corres[parc_f1->data]], parc_f1->data+1);
        parc_f0 = parc_f0->next;
        parc_f1 = parc_f1->next;
    }
    
    free_list(p);
    free_list(d);
    free_list(f[0]);
    free_list(f[1]);
    free(f);
    return a;
}

/*====================lectures dans l'automate=====================*/
/*renvoie vrai si la ligne est reconnue par l'automate a :*/
bool recognize_line(automate* a, char* line) {
    int n = length_char_array(line)-1; //-1 pour le retour à la ligne
    if(n<0) {
        return false;
    }
    bool* old_position = malloc(sizeof(bool)*a->nb_etats);
    bool* new_position = malloc(sizeof(bool)*a->nb_etats);
    for(int i = 0; i<a->nb_etats; i++) {
        old_position[i] = false;
        new_position[i] = false;
    }
    old_position[a->etat_initial] = true;
    bool* tmp;
    for(int i = 0; i<n; i++) {
        for(int j = 0; j<a->nb_etats; j++) {
            if(old_position[j]) {
                list* l = a->delta[j][(int)line[i]];
                while(l != NULL) {
                    new_position[l->data] = true;
                    l = l->next;
                }
                l = a->delta[j][(int)'.'];
                while(l != NULL) {
                    new_position[l->data] = true;
                l = l->next;
                }
                old_position[j] = false;
            }
        }
        tmp = old_position;
        old_position = new_position;
        new_position = tmp;
    }
    bool ret = false;
    for(int i = 0; i<a->nb_etats; i++) {
        if(a->etats_finaux[i] && old_position[i]) {
            ret = true;
        }
    }
    free(old_position);
    free(new_position);
    return ret;
}

/*parcours des lignes d'un fichier et affichage des lignes si elles sont reconnues par l'automate a :*/
void get_lines(automate* a, char* file_name) {
    FILE* file = fopen(file_name, "r");
    char line[1000];
    int i = 0;
    while(fgets(line, sizeof(line), file) != NULL) {
        if(recognize_line(a, line)) {
            printf("%d : %s", i, line);
        }
        i++;
    }
    fclose(file);
    return;
}

/*====================fonctions d'affichage=====================*/
/*affichage des arbres :*/
void print_tree_symbols(tree* t) {
    if(t == NULL) {
        return;
    }
    if(is_symbol(t->value)) {
        printf("%c -> (", (char)t->value);
    }
    else if(t->value == -1) {
        printf("ε -> (");
    }
    else{
        printf("%d -> (", t->value);
    }
    print_tree_symbols(t->left_child);
    if(t->right_child != NULL) {
        printf(", ");
        print_tree_symbols(t->right_child);
    }
    printf(")");
}

/*affichage des piles :*/
void print_stack_symbols(stack* s) {
    printf("[");
    for(int i = 0; i<s->size-1; i++) {
        print_tree_symbols(s->data[i]);
        printf(", ");
    }
    if(s->size != 0) {
        print_tree_symbols(s->data[s->size-1]);
    }
    printf("]\n");
}

/*affichages des listes d'entiers :*/
void print_int_list(list* l) {
    while(l != NULL) {
        printf("%d ", l->data);
        l = l->next;
    }
    printf("\n");
}

/*affichage des listes de couples :*/
void print_couple_int_list(list** l) {
    list* l0 = l[0];
    list* l1 = l[1];
    while(l0 != NULL) {
        printf("(%d, %d) ", l0->data, l1->data);
        l0 = l0->next;
        l1 = l1->next;
    }
    printf("\n");
}

/*affichage du tableau de correspondance obtenu lors de la linéarisation*/
void print_tab_corres(int* tab_corres, int size) {
    for(int i = 0; i<size; i++) {
        printf("%d : %c, ", i, (char) tab_corres[i]);
    }
    printf("\n");
}

/*affichage du résultat de la fonction parse :*/
void print_result_parse(char* s) {
    regex* re = char_array_to_regex(s);
    int* tab_corres;
    tree* t = parse(re, &tab_corres);
    print_tree_symbols(t);
    printf("\n");
    print_tab_corres(tab_corres, nb_letters(t));
    free_tree(t);
    free(tab_corres);
    free_regex(re);
    printf("\n");
}

/*affichage des ensembles P, D et F :*/
void print_pdf(char* s) {
    regex* re = char_array_to_regex(s);
    int* tab_corres;
    tree* t = parse(re, &tab_corres);
    print_tree_symbols(t);
    printf("\n");

    list* p = calc_P(t);
    list* d = calc_D(t);
    list** f= calc_F(t);
    printf("P = ");
    print_int_list(p);
    printf("D = ");
    print_int_list(d);
    printf("F = ");
    print_couple_int_list(f);
    free_list(p);
    free_list(d);
    free_list(f[0]);
    free_list(f[1]);
    free(f);
    free_tree(t);
    free(tab_corres);
    free_regex(re);
}

/*écriture de l'automate d'une expression régulière s :*/
void write_automate_of_string(char* s, char* name) {
    regex* re = char_array_to_regex(s);
    int* tab_corres;
    tree* t = parse(re, &tab_corres);
    automate* a = create_automate(t, tab_corres);
    ecrire_automate(a, name);
    free_automate(a);
    free_tree(t);
    free(tab_corres);
    free_regex(re);
}

/*====================fonction main=====================*/
int main(int argc, char* argv[]) {
    if(argc != 3) {
        return -1;
    }
    regex* re = char_array_to_regex(argv[1]);
    int* tab_corres;
    tree* t = parse(re, &tab_corres);
    automate* a = create_automate(t, tab_corres);
    get_lines(a, argv[2]);
    //ecrire_automate(a, "auto.viz");
    free_automate(a);
    free_regex(re);
    free_tree(t);
    free(tab_corres);
    return 0;
}

/*====================Tests=====================*/
/* Tests des piles :
int main() {
    tree* t;
    //stack vide :
    stack* s = init_stack();
    printf("capacité de la pile : %d\n", s->capacity);
    print_stack_symbols(s);
    //pile contenant 5 arbres de taille 1 : 0, 1, 2, 3, 4 pour doubler la taille de la pile :
    push(s, init_tree(0));
    push(s, init_tree(1));
    push(s, init_tree(2));
    push(s, init_tree(3));
    push(s, init_tree(4));
    printf("capacité de la pile : %d\n", s->capacity);
    print_stack_symbols(s);
    //on dépile 3 arbres pour diminuer la capacité de la pile :
    t = pop(s);
    free_tree(t);
    t = pop(s);
    free_tree(t);
    t = pop(s);
    free_tree(t);
    printf("capacité de la pile : %d\n", s->capacity);
    print_stack_symbols(s);
    //on change le dernier arbre pour 1?
    t = init_tree((int)'?');
    t->left_child = pop(s);
    push(s, t);
    print_stack_symbols(s);
    //on construit un arbre de racine @ et aillant comme fils les 2 arbres de la pile :
    t = init_tree((int)'@');
    t->right_child = pop(s);
    t->left_child = pop(s);
    push(s, t);
    print_stack_symbols(s);
    ////on construit un arbre de racine ? et aillant comme fils l'arbre dans la pile :
    t = init_tree((int)'?');
    t->left_child = pop(s);
    push(s, t);
    print_stack_symbols(s);
    //on dépile l'arbre :
    t = pop(s);
    print_stack_symbols(s);
    //on remplace les ? :
    t = replace_interog(t);
    print_tree_symbols(t);
    printf("\n");

    free_tree(t);
    free_stack(s);
    return 0;
}
*/
/* Tests de la fonction parse :
int main() {
    //test sur les opérateurs et les lettres :
    print_result_parse("a");
    print_result_parse("a*");
    print_result_parse("a?");
    print_result_parse("aa@");
    print_result_parse("aa|");
    //même exemple que celui détaillé au dessus mais directement avec la fonction parse :
    print_result_parse("ab?@?");
    return 0;
}
*/
/* Tests des fonctions de calculs de P, D et F :
int main() {
    print_pdf("a");
    print_pdf("a*");
    print_pdf("a?");
    print_pdf("aa@");
    print_pdf("aa|");
    return 0;
}
*/
/* Tests des créations d'automates :
int main() {
    write_automate_of_string("", "auto0.viz");
    write_automate_of_string("a", "auto1.viz");
    write_automate_of_string("a*", "auto2.viz");
    write_automate_of_string("a?", "auto3.viz");
    write_automate_of_string("aa@", "auto4.viz");
    write_automate_of_string("ab|", "auto5.viz");
    write_automate_of_string("a.?@", "auto6.viz");
}
*/
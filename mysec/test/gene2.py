import random

# Nombre de tests que nous voulons générer
NUM_TESTS = 100

# Génère un appel aléatoire à my_malloc, my_free, my_calloc, my_realloc
def generate_random_call():
    function = random.choice(['my_malloc', 'my_free', 'my_calloc', 'my_realloc'])
    if function == 'my_malloc':
        size = random.randint(1, 1000)
        return f"    ptr = {function}({size});"
    elif function == 'my_free':
        return f"    {function}(ptr);"
    elif function == 'my_calloc':
        num = random.randint(1, 100)
        size = random.randint(1, 1000)
        return f"    ptr = {function}({num}, {size});"
    elif function == 'my_realloc':
        size = random.randint(1, 1000)
        return f"    ptr = {function}(ptr, {size});"
    
# Génère le code C complet avec les appels de test
def generate_c_code(num_tests):
    c_code = """
#include <stdio.h>
#include <stdlib.h>

// Déclarations des fonctions my_malloc, my_free, my_calloc, my_realloc
void* my_malloc(size_t size);
void my_free(void* ptr);
void* my_calloc(size_t num, size_t size);
void* my_realloc(void* ptr, size_t size);

int main() {
    void* ptr = NULL;

"""
    for _ in range(num_tests):
        c_code += generate_random_call() + "\n"

    c_code += """
    return 0;
}
"""
    return c_code

# Génère le fichier C
with open('test_my_memory_functions.c', 'w') as f:
    f.write(generate_c_code(NUM_TESTS))

print("Le fichier test_my_memory_functions.c a été généré.")

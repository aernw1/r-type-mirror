# Guide de Style de Code - R-Type

Ce document décrit les conventions de style de code utilisées dans le projet R-Type. Le respect de ces règles assure la cohérence et la lisibilité du code à travers tout le projet.

## 📋 Table des matières

- [Formatage Automatique](#formatage-automatique)
- [Indentation et Espacement](#indentation-et-espacement)
- [Namespaces](#namespaces)
- [Classes et Structures](#classes-et-structures)
- [Fonctions](#fonctions)
- [Pointeurs et Références](#pointeurs-et-références)
- [Includes](#includes)
- [Vérification du Format](#vérification-du-format)

---

## 🔧 Formatage Automatique

Le projet utilise **clang-format** pour garantir un formatage cohérent du code C++. La configuration se trouve dans le fichier `.clang-format` à la racine du projet.

### Utilisation

```bash
# Formater un fichier
clang-format -i fichier.cpp

# Formater tous les fichiers du projet
find include src -name "*.hpp" -o -name "*.cpp" | xargs clang-format -i

# Vérifier sans modifier
clang-format --dry-run --Werror fichier.cpp
```

---

## 📏 Indentation et Espacement

### Règles d'indentation

- **Indentation** : 4 espaces (pas de tabulations)
- **Limite de ligne** : 100 caractères maximum
- **Lignes vides** : Maximum 2 lignes vides consécutives

```cpp
// ✅ BON
void maFonction() {
    int x = 42;
    if (x > 0) {
        std::cout << x << std::endl;
    }
}

// ❌ MAUVAIS (tabulations)
void maFonction() {
	int x = 42;  // Tabulation au lieu d'espaces
}
```

### Configuration .clang-format
```yaml
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 100
MaxEmptyLinesToKeep: 2
```

---

## 📦 Namespaces

### Structure des namespaces

Les namespaces sont **toujours indentés** et peuvent contenir des lignes vides pour améliorer la lisibilité.

```cpp
// ✅ BON
namespace RType {

    namespace ECS {
        using Entity = uint32_t;
        constexpr Entity NULL_ENTITY = 0;
    }

}
```

### Configuration .clang-format
```yaml
NamespaceIndentation: All          # Indente tout le contenu des namespaces
CompactNamespaces: false           # Permet les lignes vides entre namespaces
FixNamespaceComments: false        # N'ajoute pas de commentaires automatiques
KeepEmptyLinesAtTheStartOfBlocks: true  # Garde les lignes vides après '{'
```

---

## 🏗️ Classes et Structures

### Déclaration

- Les accolades ouvrantes sont **sur la même ligne** que la déclaration
- L'indentation des modificateurs d'accès est **au même niveau** que la classe
- Les membres sont indentés de 4 espaces

```cpp
// ✅ BON
struct Position : public IComponent {
    float x = 0.0f;
    float y = 0.0f;

    Position() = default;
    Position(float x, float y) : x(x), y(y) {}
};

class MyClass {
public:
    void doSomething();

private:
    int value;
};
```

### Héritage

Un espace est ajouté **avant et après** les deux-points d'héritage.

```cpp
// ✅ BON
struct Position : public IComponent {
    // ...
};

// ❌ MAUVAIS
struct Position: public IComponent {  // Pas d'espace avant ':'
    // ...
};
```

### Configuration .clang-format
```yaml
BreakBeforeBraces: Attach          # Accolade sur la même ligne
AccessModifierOffset: -4           # Modificateurs au même niveau que la classe
SpaceBeforeInheritanceColon: true  # Espace avant ':' dans l'héritage
```

---

## 🔨 Fonctions

### Déclaration et définition

```cpp
// ✅ BON - Fonction courte sur une ligne
Position() = default;
virtual ~IComponent() = default;

// ✅ BON - Fonction normale
void process() {
    // code
}

// ✅ BON - Constructeur avec liste d'initialisation
Position(float x, float y) : x(x), y(y) {}

// ✅ BON - Liste d'initialisation longue
ComplexClass(int a, int b, int c)
    : memberA(a), memberB(b), memberC(c) {
    // code
}
```

### Espacement avec parenthèses

- **Espace avant les parenthèses** : uniquement pour les structures de contrôle (`if`, `for`, `while`)
- **Pas d'espace** : pour les appels de fonction

```cpp
// ✅ BON
if (condition) {          // Espace avant '(' pour 'if'
    doSomething();        // Pas d'espace pour l'appel de fonction
}

for (int i = 0; i < 10; i++) {  // Espace avant '(' pour 'for'
    // code
}

// ❌ MAUVAIS
if(condition) {           // Manque l'espace
    doSomething ();       // Espace en trop
}
```

### Configuration .clang-format
```yaml
AllowShortFunctionsOnASingleLine: All     # Permet les fonctions courtes sur une ligne
AllowShortIfStatementsOnASingleLine: Never  # Interdit les 'if' sur une ligne
AllowShortLoopsOnASingleLine: false       # Interdit les boucles sur une ligne
SpaceBeforeParens: ControlStatements      # Espace uniquement pour if/for/while
BreakConstructorInitializers: BeforeColon # ':' sur la ligne du constructeur
ConstructorInitializerIndentWidth: 4      # Indentation des initialiseurs
SpaceBeforeCtorInitializerColon: true     # Espace avant ':' des initialiseurs
```

---

## 👉 Pointeurs et Références

L'**astérisque** (`*`) et le **esperluette** (`&`) sont collés au **type**, pas au nom de la variable.

```cpp
// ✅ BON
int* ptr;
const std::string& name;
void process(Entity* entity);

// ❌ MAUVAIS
int *ptr;           // Astérisque collé au nom
int * ptr;          // Espaces des deux côtés
const std::string &name;  // Esperluette collé au nom
```

### Configuration .clang-format
```yaml
PointerAlignment: Left    # * et & collés au type
```

---

## 📚 Includes

### Ordre des includes

L'ordre des `#include` est **libre** et ne sera **pas réorganisé** automatiquement par clang-format.

```cpp
#pragma once

#include <string>
#include <cstdint>
#include <typeindex>
#include <type_traits>

namespace RType {
    // ...
}
```

### Configuration .clang-format
```yaml
SortIncludes: false    # Ne réorganise pas les includes
```

---

## ✅ Formatage Automatique dans la CI/CD

### GitHub Actions - Auto-formatage

Le projet utilise un **formatage automatique** dans la pipeline CI/CD. Chaque push et pull request déclenche un formatage automatique du code.

**Comment ça marche ?**
1. Vous pushez votre code (même s'il n'est pas formaté)
2. GitHub Actions lance automatiquement `clang-format -i` sur tous vos fichiers
3. Si des changements sont nécessaires, ils sont automatiquement committés
4. Vous devez ensuite pull les changements : `git pull`

Le workflow se trouve dans `.github/workflows/clang-format-check.yml` :

```yaml
name: clang-format Auto-Fix
on:
  push:
    branches:
      - main
      - dev
  pull_request:

jobs:
  format:
    name: Auto-format Code
    runs-on: ubuntu-latest
    permissions:
      contents: write
    steps:
      - uses: actions/checkout@v4
      
      - name: Install clang-format
        run: sudo apt-get update && sudo apt-get install -y clang-format
      
      - name: Format code with clang-format
        run: |
          find include src -type f \( -name "*.hpp" -o -name "*.cpp" \) -exec clang-format -i {} +
      
      - name: Commit formatted code (si nécessaire)
        run: |
          git config user.name "github-actions[bot]"
          git add .
          git commit -m "style: auto-format code" || exit 0
          git push
```

### ⚠️ Important

Après un push, si le code a été formaté automatiquement par GitHub Actions, n'oubliez pas de récupérer les changements :

```bash
git pull
```


# Ou manuellement
find include src -type f \( -name "*.hpp" -o -name "*.cpp" \) -exec clang-format -i {} +
```

### Pré-commit hook (optionnel)

Vous pouvez créer un hook git pour formater automatiquement avant chaque commit :

```bash
# Créer le fichier .git/hooks/pre-commit
cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
# Format all staged C++ files
for file in $(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|hpp)$'); do
    clang-format -i "$file"
    git add "$file"
done
EOF

# Rendre le hook exécutable
chmod +x .git/hooks/pre-commit
```

---

## 📖 Récapitulatif de la Configuration

Voici le fichier `.clang-format` complet avec explications :

```yaml
---
Language: Cpp
BasedOnStyle: LLVM                          # Base LLVM avec personnalisations

# Indentation
IndentWidth: 4                              # 4 espaces par niveau
TabWidth: 4                                 # Tabulation = 4 espaces (non utilisé)
UseTab: Never                               # Jamais de tabulations, toujours des espaces

# Lignes
ColumnLimit: 100                            # Max 100 caractères par ligne
MaxEmptyLinesToKeep: 2                      # Max 2 lignes vides consécutives
KeepEmptyLinesAtTheStartOfBlocks: true      # Garde les lignes vides après '{'

# Namespaces
NamespaceIndentation: All                   # Indente tout le contenu des namespaces
CompactNamespaces: false                    # Permet les lignes vides entre namespaces
FixNamespaceComments: false                 # N'ajoute pas de commentaires automatiques

# Accolades
BreakBeforeBraces: Attach                   # Accolades sur la même ligne (style K&R)

# Fonctions
AllowShortFunctionsOnASingleLine: All       # Fonctions courtes sur une ligne OK
AllowShortIfStatementsOnASingleLine: Never  # 'if' toujours multiligne
AllowShortLoopsOnASingleLine: false         # Boucles toujours multiligne

# Parenthèses et espaces
SpaceBeforeParens: ControlStatements        # Espace avant '(' seulement pour if/for/while

# Pointeurs et références
PointerAlignment: Left                      # '*' et '&' collés au type

# Classes
AccessModifierOffset: -4                    # public:/private: au niveau de la classe
EmptyLineBeforeAccessModifier: Never        # Pas de ligne vide avant public:/private:

# Constructeurs
BreakConstructorInitializers: BeforeColon   # ':' sur la ligne du constructeur
ConstructorInitializerIndentWidth: 4        # Indentation des listes d'initialisation
SpaceBeforeCtorInitializerColon: true       # Espace avant ':' des initialiseurs
SpaceBeforeInheritanceColon: true           # Espace avant ':' de l'héritage

# Includes
SortIncludes: false                         # Ne réorganise PAS les includes

# Switch
IndentCaseLabels: false                     # 'case' au même niveau que 'switch'
```

---

## 🎓 Exemples Complets

### Exemple de fichier header complet

```cpp
#pragma once

#include <string>
#include <cstdint>
#include <typeindex>
#include <type_traits>

namespace RType {

    namespace ECS {
        using ComponentID = std::type_index;

        struct IComponent {
            virtual ~IComponent() = default;
        };

        struct Position : public IComponent {
            float x = 0.0f;
            float y = 0.0f;

            Position() = default;
            Position(float x, float y) : x(x), y(y) {}

            void move(float dx, float dy) {
                x += dx;
                y += dy;
            }
        };

        struct Velocity : public IComponent {
            float dx = 0.0f;
            float dy = 0.0f;

            Velocity() = default;
            Velocity(float dx, float dy) : dx(dx), dy(dy) {}
        };
    }

}
```

---

## 📞 Support

En cas de questions sur le style de code ou des problèmes avec clang-format :

1. Vérifiez que vous avez la bonne version : `clang-format --version` (13.x)
2. Assurez-vous que le fichier `.clang-format` est à la racine du projet
3. Consultez la documentation officielle : [Clang-Format Style Options](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)

---

**Dernière mise à jour** : Novembre 2025  
**Version clang-format** : 13.0.1


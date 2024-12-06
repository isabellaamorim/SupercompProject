#include <iostream>
#include <vector>
#include <fstream>
#include <chrono> 

using namespace std;

// Do enunciado do projeto: Função para ler o grafo a partir do arquivo de entrada
std::vector<std::vector<int>> LerGrafo(const std::string& nomeArquivo, int& numVertices) {
    std::ifstream arquivo(nomeArquivo);
    int numArestas;
    arquivo >> numVertices >> numArestas;

    std::vector<std::vector<int>> grafo(numVertices, std::vector<int>(numVertices, 0));

    for (int i = 0; i < numArestas; ++i) {
        int u, v;
        arquivo >> u >> v;
        grafo[u - 1][v - 1] = 1;
        grafo[v - 1][u - 1] = 1;  // O grafo é não direcionado
    }

    arquivo.close();

    return grafo;
}

void CliqueMaximaExaustiva(vector<vector<int>>& grafo, vector<int>& cliqueAtual, vector<int>& candidatos, vector<int>& cliqueMaxima) {
    // Se não houver mais candidatos, a recursão termina
    if (candidatos.empty()) {
        if (cliqueAtual.size() > cliqueMaxima.size()) {
            cliqueMaxima = cliqueAtual;
        }
        return;
    }

    // Loop sobre os candidatos
    for (size_t i = 0; i < candidatos.size(); ++i) {
        int v = candidatos[i];

        // Verifica se v é adjacente a todos em CliqueAtual
        // Objetivo: apenas adiciona v a clique se ele for adjacente a todos os vértices em CliqueAtual
        bool adjacenteATodos = true;
        for (int u : cliqueAtual) {
            if (grafo[u][v] == 0) {
                adjacenteATodos = false;
                break;
            }
        }

        // Expansão da CliqueAtual
        if (adjacenteATodos) {
            // Adiciona v a cliqueAtual, se for adjacente a todos
            cliqueAtual.push_back(v);

            // Atualizar a cliqueMaxima se cliqueAtual for maior
            if (cliqueAtual.size() > cliqueMaxima.size()) {
                cliqueMaxima = cliqueAtual;
            }

            // Atualização da lista de candidatos
            vector<int> novosCandidatos;
            // Itera após V para evitar duplicação
            for (size_t j = i + 1; j < candidatos.size(); ++j) {
                int w = candidatos[j];

                // Verificar se w é adjacente a todos os vértices em cliqueAtual
                bool adjacenteANovos = true;
                for (int u : cliqueAtual) {
                    if (grafo[u][w] == 0) {
                        adjacenteANovos = false;
                        break;
                    }
                }

                if (adjacenteANovos) {
                    novosCandidatos.push_back(w);
                }
            }

            // Chamada recursiva
            CliqueMaximaExaustiva(grafo, cliqueAtual, novosCandidatos, cliqueMaxima);

            // Remover v da cliqueAtual
            cliqueAtual.pop_back();
        }
    }
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        cout << "Uso: " << argv[0] << " <arquivo_de_entrada>" << endl;
        return 1;
    }

    auto start = std::chrono::high_resolution_clock::now();

    int numVertices;

    string grafoArquivo = argv[1];

    vector<vector<int>> grafo = LerGrafo(grafoArquivo, numVertices);

    // Inicializar candidatos com todos os vértices
    vector<int> candidatos(numVertices);
    for (int i = 0; i < numVertices; ++i) {
        candidatos[i] = i;
    }

    vector<int> cliqueAtual;
    vector<int> cliqueMaxima;

    CliqueMaximaExaustiva(grafo, cliqueAtual, candidatos, cliqueMaxima);

    cout << "Clique max - Busca Exaustiva: ";
    for (int v : cliqueMaxima) {
        cout << (v + 1) << " ";
    }
    cout << endl;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Tempo de exec. - Exaustiva: " << duration.count() << " segundos" << std::endl;

    return 0;
}

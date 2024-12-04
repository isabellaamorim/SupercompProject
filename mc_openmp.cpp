#include <omp.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <algorithm>
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

int main(int argc, char* argv[]) {

    auto start = std::chrono::high_resolution_clock::now();

    // Para casos sem input
    if (argc < 2) {
        cout << "Input: executavel grafo.txt" << endl;
        return 1;
    }

    int numVertices;

    string grafoArquivo = argv[1];

    vector<vector<int>> grafo = LerGrafo(grafoArquivo, numVertices);

    vector<int> cliqueMaxima;

    // Paraleliza sobre os vértices iniciais
    #pragma omp parallel
    {
        // Clique máxima exclusiva por thread
        vector<int> cliqueMaximaLocal;

        #pragma omp for schedule(dynamic)
        for (int i = 0; i < numVertices; ++i) {

            vector<int> cliqueAtual;
            cliqueAtual.push_back(i);
            vector<int> candidatos;

            // Inicializa candidatos com os vizinhos de i
            for (int j = 0; j < numVertices; ++j) {
                if (grafo[i][j] == 1 && j > i) { 
                    candidatos.push_back(j);
                }
            }

            // Usando uma pilha para simular a recursão
            stack<pair<vector<int>, vector<int>>> pilha;
            pilha.push({cliqueAtual, candidatos});

            while (!pilha.empty()) {
                auto estado = pilha.top();
                pilha.pop();

                vector<int> clique = estado.first; //Clique atual
                vector<int> candidates = estado.second; //Candidatos

                //Verifica se a pilha está vazia
                if (candidates.empty()) {
                    if (clique.size() > cliqueMaximaLocal.size()) {
                        cliqueMaximaLocal = clique;
                    }
                    continue;
                }

                // Loop sobre os candidatos
                for (size_t idx = 0; idx < candidates.size(); ++idx) {
                    int v = candidates[idx];

                    // Verifica se v é adjacente a todos em clique
                    // Objetivo: apenas adiciona v a clique se ele for adjacente a todos os vértices em clique
                    bool adjacenteATodos = true;
                    for (int u : clique) {
                        if (grafo[u][v] == 0) {
                            adjacenteATodos = false;
                            break;
                        }
                    }

                    // Expansão da clique
                    if (adjacenteATodos) {

                        // Adiciona v a clique, se for adjacente a todos
                        vector<int> novoClique = clique;
                        novoClique.push_back(v);

                        // Atualização da lista de candidatos
                        vector<int> novosCandidatos;
                        // Itera após V para evitar duplicação
                        for (size_t j = idx + 1; j < candidates.size(); ++j) {
                            int w = candidates[j];
                            // Verifica se w é adjacente a v, se sim, adiciona w a novosCandidatos
                            if (grafo[v][w] == 1) {
                                novosCandidatos.push_back(w);
                            }
                        }

                        // Empilha novoClique e novosCandidatos
                        pilha.push({novoClique, novosCandidatos});

                        if (novoClique.size() > cliqueMaximaLocal.size()) {
                            cliqueMaximaLocal = novoClique;
                        }
                    }
                }
            }
        }

        // Atualiza a clique máxima global
        #pragma omp critical
        {
            if (cliqueMaximaLocal.size() > cliqueMaxima.size()) {
                cliqueMaxima = cliqueMaximaLocal;
            }
        }
    }

    cout << "Clique max - OpenMP: ";
    for (int v : cliqueMaxima) {
        cout << (v + 1) << " ";
    }
    cout << endl;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Tempo de exec. - OpenMP: " << duration.count() << " segundos" << std::endl;

    return 0;
}

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <algorithm>
#include <chrono>

using namespace std;

// Função para ler o grafo a partir do arquivo de entrada
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

    // Inicializar o ambiente MPI
    MPI_Init(&argc, &argv);

    // Obter o rank e o número de processos
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Variáveis para medição de tempo
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

    // Apenas o processo mestre registra o tempo inicial
    if (rank == 0) {
        start = std::chrono::high_resolution_clock::now();
    }

    int numVertices;
    vector<vector<int>> grafo;

    // Para casos sem input
    if (argc < 2) {
        if (rank == 0) {
            cout << "Uso: executavel grafo.txt" << endl;
        }
        MPI_Finalize();
        return 1;
    }

    string grafoArquivo = argv[1];

    // Rank 0 lê o grafo
    if (rank == 0) {
        grafo = LerGrafo(grafoArquivo, numVertices);
    }

    // Broadcast do número de vértices para todos os processos
    MPI_Bcast(&numVertices, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Broadcast da matriz de adjacência para todos os processos
    if (rank != 0) {
        grafo.resize(numVertices, vector<int>(numVertices, 0));
    }
    for (int i = 0; i < numVertices; ++i) {
        MPI_Bcast(&grafo[i][0], numVertices, MPI_INT, 0, MPI_COMM_WORLD);
    }

    vector<int> cliqueMaximaLocal;

    // Dividir os vértices entre os processos
    vector<int> verticesLocais;
    for (int i = rank; i < numVertices; i += size) {
        verticesLocais.push_back(i);
    }

    // Cada processo encontra a clique máxima a partir dos vértices locais
    for (int i : verticesLocais) {
        vector<int> cliqueAtual;
        cliqueAtual.push_back(i);
        vector<int> candidatos;
        // Inicializa candidatos com os vizinhos de i
        for (int j = 0; j < numVertices; ++j) {
            if (grafo[i][j] == 1 && j > i) { // Para evitar duplicação
                candidatos.push_back(j);
            }
        }

        // Usando uma pilha para simular a recursão
        stack<pair<vector<int>, vector<int>>> pilha;
        pilha.push({cliqueAtual, candidatos});

        while (!pilha.empty()) {
            auto estado = pilha.top();
            pilha.pop();

            vector<int> clique = estado.first; // Clique atual
            vector<int> candidates = estado.second; // Candidatos

            // Verifica se não há mais candidatos
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
                bool adjacenteATodos = true;
                for (int u : clique) {
                    if (grafo[u][v] == 0) {
                        adjacenteATodos = false;
                        break;
                    }
                }

                // Expansão da clique
                if (adjacenteATodos) {

                    // Adiciona v à clique
                    vector<int> novoClique = clique;
                    novoClique.push_back(v);

                    // Atualização da lista de candidatos
                    vector<int> novosCandidatos;
                    // Itera após v para evitar duplicação
                    for (size_t j = idx + 1; j < candidates.size(); ++j) {
                        int w = candidates[j];
                        // Verifica se w é adjacente a v
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

    // Rank 0 recolhe as cliques máximas locais dos demais processos
    int tamanhoCliqueLocal = cliqueMaximaLocal.size();

    // Enviar os tamanhos das cliques máximas locais para o processo mestre
    vector<int> tamanhosClique(size);
    MPI_Gather(&tamanhoCliqueLocal, 1, MPI_INT, &tamanhosClique[0], 1, MPI_INT, 0, MPI_COMM_WORLD);

    vector<int> cliqueMaximaGlobal;

    if (rank == 0) {
        vector<vector<int>> cliquesLocais(size);

        // Clique máxima local do rank 0 (mestre)
        cliquesLocais[0] = cliqueMaximaLocal;

        // Receber as cliques máximas dos demais processos
        for (int i = 1; i < size; ++i) {
            int tamanho = tamanhosClique[i];
            vector<int> cliqueRecebida(tamanho);
            MPI_Recv(&cliqueRecebida[0], tamanho, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            cliquesLocais[i] = cliqueRecebida;
        }

        // Determinar a clique máxima global
        for (int i = 0; i < size; ++i) {
            if (cliquesLocais[i].size() > cliqueMaximaGlobal.size()) {
                cliqueMaximaGlobal = cliquesLocais[i];
            }
        }

        cout << "Clique máxima encontrada: ";
        for (int v : cliqueMaximaGlobal) {
            cout << (v + 1) << " ";
        }
        cout << endl;
    } else {
        // Enviar a clique máxima local para o processo mestre
        MPI_Send(&cliqueMaximaLocal[0], tamanhoCliqueLocal, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    // Sincronizar todos os processos antes de medir o tempo final
    MPI_Barrier(MPI_COMM_WORLD);

    // Apenas o processo mestre registra o tempo final e imprime a duração
    if (rank == 0) {
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Tempo de execução - MPI: " << duration.count() << " segundos" << std::endl;
    }

    MPI_Finalize();

    return 0;
}

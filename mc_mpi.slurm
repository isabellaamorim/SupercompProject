#!/bin/bash
#SBATCH --job-name=mpi            
#SBATCH --output=mc_mpi_output_6.log    
#SBATCH --ntasks=8                    
#SBATCH --cpus-per-task=1
#SBATCH --mem=4G                  
#SBATCH --time=00:15:00               
#SBATCH --partition=espec         

# Executar o programa
./mc_mpi "grafo6.txt"

#!/bin/bash
#SBATCH --job-name=mc_openmp             
#SBATCH --output=mc_openmp_output_6.log   
#SBATCH --ntasks=1                    
#SBATCH --cpus-per-task=8
#SBATCH --mem=1024                  
#SBATCH --time=00:05:00               
#SBATCH --partition=espec        

# Executar o programa
./mc_openmp "grafo6.txt"

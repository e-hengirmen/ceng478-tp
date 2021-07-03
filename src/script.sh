mpicc solution.c -o parallel -lm
gcc serial.c -o sequential -lm
echo "Compiled"
mpirun -np 9 ./parallel
./sequential <input
# echo "diff is executed for outputs of parallel and sequential algorithms below"
# echo "if you see any differences it means that one of the algoritms are wrong"
echo "differences are:"
diff output output2
# echo ""
# echo "Initial matrices(A and B) of matrix multiplication has been stored in input file"
# echo "Output matrices from parallel and sequential codes are stored in output and output2 files"

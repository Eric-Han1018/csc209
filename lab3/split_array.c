#include <stdio.h>
#include <stdlib.h>

/* Return a pointer to an array of two dynamically allocated arrays of ints.
   The first array contains the elements of the input array s that are
   at even indices.  The second array contains the elements of the input
   array s that are at odd indices.

   Do not allocate any more memory than necessary.
*/
int **split_array(const int *s, int length) {
  int **nums = malloc(2*sizeof(int*));
  int half = length/2;
  int odd = 0;
  int even = 0;
  nums[0] = malloc((length-half)*sizeof(int));
  nums[1] = malloc(half*sizeof(int));
  for(int i = 0; i<length; i++){
    if((i+1)%2){
      nums[0][odd] = s[i];
      odd += 1;
    }
    else{
      nums[1][even] = s[i];
      even += 1;
    }
  }
  return nums;
}

/* Return a pointer to an array of ints with size elements.
   - strs is an array of strings where each element is the string
     representation of an integer.
   - size is the size of the array
 */

int *build_array(char **strs, int size) {
  int *result = malloc(size*sizeof(int));
  for(int i = 0; i<size; i++){
    result[i] = strtol(strs[i+1], NULL,10);
  }
  return result;
}


int main(int argc, char **argv) {
    /* Replace the comments in the next two lines with the appropriate
       arguments.  Do not add any additional lines of code to the main
       function or make other changes.
     */
    int *full_array = build_array(argv, argc-1);
    int **result = split_array(full_array, argc-1);

    printf("Original array:\n");
    for (int i = 0; i < argc - 1; i++) {
        printf("%d ", full_array[i]);
    }
    printf("\n");

    printf("result[0]:\n");
    for (int i = 0; i < argc / 2; i++) {
        printf("%d ", result[0][i]);
    }
    printf("\n");

    printf("result[1]:\n");
    for (int i = 0; i < (argc - 1) / 2; i++) {
        printf("%d ", result[1][i]);
    }
    printf("\n");
    free(full_array);
    free(result[0]);
    free(result[1]);
    free(result);
    return 0;
}


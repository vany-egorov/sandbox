from data import data

def cocktail_sort(A):
    for k in range(len(A)-1, 0, -1):
        swapped = False
        for i in range(k, 0, -1):
            if A[i]<A[i-1]:
                A[i], A[i-1] = A[i-1], A[i]
                swapped = True
 
        for i in range(k):
            if A[i] > A[i+1]:
                A[i], A[i+1] = A[i+1], A[i]
                swapped = True
 
        if not swapped:
            return A

cocktail_sort(data)
print(data)

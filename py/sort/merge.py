from data import data

def merge_sort(li):
    if len(li) < 2:
        return li
    m = len(li) / 2
    return merge(merge_sort(li[:int(m)]), merge_sort(li[int(m):]))

def merge(l, r):
    print(l, r)
    result = []
    i = j = 0
    while i < len(l) and j < len(r):
        if l[i] < r[j]:
            result.append(l[i])
            i += 1
        else:
            result.append(r[j])
            j += 1            
    result += l[i:]
    result += r[j:]
    return result

data = merge_sort(data)
print(data)

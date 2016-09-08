from data import data

def bubble(bad_list):
    length = len(bad_list) - 1
    sorted = False

    while not sorted:
        sorted = True
        for i in range(length):
            if bad_list[i] > bad_list[i+1]:
                sorted = False
                bad_list[i], bad_list[i+1] = bad_list[i+1], bad_list[i]

bubble(data)
print(data)

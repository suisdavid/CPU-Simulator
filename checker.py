import subprocess
name_list=['array_test1','array_test2','basicopt1','bulgarian','expr','gcd','hanoi','lvalue2','magic','manyarguments','multiarray','naive','pi','qsort','queens','statement_test','superloop','tak']
ans=[123,43,88,159,58,178,20,175,106,40,115,94,137,105,171,50,134,186]
n=len(name_list)#18
passed=0
for i in range(n):
    file_name='testcases/'+name_list[i]+'.data'
    with open(file_name,'r') as f:
        data=f.read()
    try:
        result = subprocess.run('./code', input=data, capture_output=True, text=True, timeout=30)
        if int(result.stdout)!=ans[i]:
            print(f"{name_list[i]} FAILED!")
        else:
            print(f"{name_list[i]} PASSED!")
            passed+=1
    except:
        print(f"{name_list[i]} FAILED!")
print(f"Total passed={passed}/18")
   
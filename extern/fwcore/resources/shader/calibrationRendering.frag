#version 330 core
in vec3 v_TexCoord;

out vec4 color;

// Texture samplers
uniform sampler2D tex;
uniform sampler2D alphaOverlap;
uniform sampler2D alphaTrans;

uniform sampler2DArray colorLookup;

uniform vec2 resolution;
uniform bool withAlphaTrans;

void main()
{
    float gamma = 1.0/2.2;
    // Converting (x,y,z) to range [0,1]
    float x = gl_FragCoord.x / resolution.x; 
    float y = gl_FragCoord.y / resolution.y; 
    float z = gl_FragCoord.z; // Already in range [0,1]

    // Converting from range [0,1] to NDC [-1,1]
    float ndcx = x * 2.0 - 1.0;
    float ndcy = y * 2.0 - 1.0;
    float ndcz = z * 2.0 - 1.0;

    // Linearly interpolate between both textures (second texture is only slightly combined)

    //evtl hier noch die textur abgreifen und alpha wert rausfinden
    //color = vec4(texture(tex, outTexCoord.xy).rgb, 1.0f);
    //color = vec4(outTexCoord.xy, 1.0, 1.0);

    vec2 coord = vec2(v_TexCoord.s / v_TexCoord.p, v_TexCoord.t / v_TexCoord.p);
    vec4 colorTex = texture(tex, coord.xy);	

    vec2 alphaCoord = vec2(x, y);
    //vec2 alphaCoord1 = (gl_FragCoord.xy + 1.0) / 2.0;
    float alphaOverlapValue = texture(alphaOverlap, alphaCoord).r;

    if(withAlphaTrans) {
        //Test1 => if (overlaps <= 2.0) then alphaTras else alphaOerlap
        if(alphaOverlapValue >= 0.7 && alphaOverlapValue < 1.0) {
            float alphaTransValue = texture(alphaTrans, alphaCoord).r;
            alphaOverlapValue = pow(alphaTransValue, gamma);
        }	
            
        /*
        //Test2 => AlphaTrans_Idee.txt
        //TODO: test with and without blur and colorCalib
        if(alphaOverlapValue < 1.0) { 
            float alphaTransValue = texture(alphaTrans, alphaCoord).r;	
            //float alphaOverlapValue = texture(alphaOverlap, alphaCoord).r;
            if(alphaOverlapValue < 0.7) { // => if(overlaps=2) then (alphaOverlapValue=0,72974)
                float div = pow(alphaOverlapValue, 2.2); //z.B. overlaps=4 => alphaOverlapValue=0,53252 => div=0,25
                float overlaps = 1.0 / div;
                //if(overlaps > 2.0) { //kann auskomentiert werden => ergibt sich aus (alphaOverlapValue < 0.7)
                    float transVertexVal = pow(0.5, overlaps - 1.0);
                    float transOverlaps = 1.0 / transVertexVal;
                    alphaTransValue = alphaTransValue * transOverlaps / overlaps;
                //}
            }
            alphaOverlapValue = pow(alphaTransValue, gamma);
        }
        */
    }

    //uint8_t
    int indexR = int(colorTex.x*255);
    int indexG = int(colorTex.y*255);
    int indexB = int(colorTex.z*255);

    //x and y in range [0,1]
    // float colR = texture(colorLookup, vec3(alphaCoord, colorTex.r * 256)).r; //colorTex.r * 256
    // float colG = texture(colorLookup, vec3(alphaCoord, colorTex.g * 256)).g;
    // float colB = texture(colorLookup, vec3(alphaCoord, colorTex.b * 256)).b;
    // vec4 correctedColor = vec4(colR, colG, colB, colorTex.a); // werte sind bereits durch 255 geteilt
    correctedColor = colorTex;
    color = correctedColor;
    
    //color = mix(vec4(0, 0, 0, 1), correctedColor,  alphaOverlapValue); 

    //color = mix(vec4(0, 0, 0, 1), texture(tex, coord.xy),  alphaTransValue);
    //color = mix(vec4(0, 0, 0, 1), texture(tex, coord.xy),  alphaOverlapValue);
}


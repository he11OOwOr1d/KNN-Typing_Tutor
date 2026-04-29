#ifndef DICTIONARY_H
#define DICTIONARY_H

// ─── EASY: Short, common words (160 words) ───
static const char* words_easy[] = {
    "the","of","and","to","in","a","is","that","for","it",
    "as","was","with","be","by","on","not","he","i","this",
    "are","or","his","from","at","but","have","an","had","they",
    "you","were","one","all","we","can","her","has","there","been",
    "if","more","when","will","who","so","no","she","do","up",
    "out","then","them","some","him","than","time","very","your","me",
    "know","just","come","its","over","also","each","set","want","well",
    "take","only","good","new","look","way","use","day","find","here",
    "give","most","hand","high","help","line","too","any","right","old",
    "keep","big","end","must","home","turn","read","last","us","run",
    "need","own","get","say","go","see","how","now","much","make",
    "year","what","did","two","our","put","life","down","same","man",
    "back","long","made","may","call","few","hard","talk","walk","eye",
    "plan","dark","ship","true","blue","deep","best","cold","fast","hold",
    "step","rain","note","fire","rock","warm","girl","team","game","lay",
    "wall","test","rest","gold","cut","hot","deal","cost","draw","free"
};
#define EASY_COUNT 160

// ─── MEDIUM: Mixed length, common vocabulary (170 words) ───
static const char* words_medium[] = {
    "about","many","these","could","first","water","into","after",
    "think","tell","does","three","little","work","people","place",
    "great","before","even","still","mean","never","move","live",
    "play","point","form","food","between","state","change","light",
    "write","follow","stop","city","tree","begin","while","world",
    "next","near","head","stand","page","open","start","might",
    "story","young","sound","house","letter","small","number","show",
    "every","side","part","kind","name","school","should","father",
    "land","sure","real","build","group","once","along","close",
    "plant","answer","learn","study","found","river","again","child",
    "face","door","woman","book","family","second","late","morning",
    "night","enough","always","music","table","money","order","watch",
    "room","large","system","often","until","early","body","power",
    "earth","mother","below","care","area","mind","voice","horse",
    "heart","result","toward","north","south","field","whole","animal",
    "mark","class","front","black","white","green","road","short",
    "glass","human","space","ground","reach","ocean","cross","drive",
    "record","boat","common","summer","winter","spring","bring","check",
    "shape","push","pull","miss","round","simple","stay","press",
    "cover","store","figure","sleep","final","touch","happy","serve",
    "strong","break","sense","clean","fresh","quiet","level","above"
};
#define MEDIUM_COUNT 168

// ─── HARD: Longer, trickier words (160 words) ───
static const char* words_hard[] = {
    "country","example","picture","problem","thought","paper","direct",
    "charge","garden","please","certain","special","window","matter",
    "ready","produce","million","among","heavy","board","cause",
    "return","corner","floor","season","machine","moment","length",
    "street","product","dream","natural","course","speak","force",
    "block","stone","notice","square","company","usual","appear",
    "train","steel","weather","weight","trade","position","travel",
    "paint","supply","island","market","report","surface","offer",
    "remain","speed","share","total","raise","surprise","captain",
    "piece","energy","silver","broad","except","thick","chance",
    "sight","crowd","exact","danger","station","branch","modern",
    "promise","master","equal","silent","divide","anger","claim",
    "score","scale","cloud","design","engine","finger","valley",
    "coast","bridge","prize","desert","signal","manage","single",
    "enjoy","price","spread","double","quarter","measure","original",
    "complete","practice","separate","difficult","sentence","opposite",
    "paragraph","consider","interest","remember","question","possible",
    "mountain","important","different","together","children","thousand",
    "discover","material","suddenly","industry","continue","electric",
    "distance","straight","determine","recognize","condition","describe",
    "molecule","exercise","indicate","property","shoulder","probably",
    "indicate","multiply","syllable","neighbor","century","consonant",
    "triangle","fraction","populate","require","molecule","physical",
    "language","familiar","solution","oxygen","hydrogen","molecule",
    "electric","magnetic","frequent","immediate","familiar","molecule"
};
#define HARD_COUNT 160

// ─── ALL words combined (for backward compat) ───
static const char* words[] = {
    "the","of","and","to","in","a","is","that","for","it",
    "as","was","with","be","by","on","not","he","i","this",
    "are","or","his","from","at","which","but","have","an","had",
    "they","you","were","their","one","all","we","can","her","has",
    "there","been","if","more","when","will","would","who","so","no",
    "she","do","about","up","out","many","then","them","these","some",
    "him","could","than","first","water","into","long","made","after","back",
    "time","very","your","me","know","other","just","come","its","over",
    "think","also","each","tell","does","set","three","want","well","take",
    "only","little","work","good","new","look","people","way","use","day",
    "find","here","thing","give","most","hand","high","place","great","help",
    "line","before","even","too","any","right","still","mean","old","keep",
    "left","big","end","must","home","under","turn","read","last","never",
    "us","move","live","play","run","need","own","point","form","food",
    "between","state","change","light","write","follow","ask","stop","city","tree",
    "begin","while","world","next","near","head","stand","page","open","start",
    "might","story","young","sound","house","letter","small","number","show","every",
    "side","part","kind","name","school","should","went","father","land","sure",
    "real","build","group","once","call","along","few","close","plant","hard",
    "answer","country","learn","study","found","river","again","walk","talk","may",
    "get","say","go","see","how","now","much","make","where","year",
    "what","did","two","when","our","put","life","down","same","man",
    "going","child","face","door","another","woman","book","family","second","late",
    "morning","night","enough","always","music","table","money","order","watch","room",
    "large","system","car","often","until","problem","air","thought","paper","early",
    "body","power","earth","water","mother","below","care","example","area","mind",
    "voice","horse","picture","heart","result","toward","north","south","east","west",
    "half","less","feet","animal","fish","whole","river","field","main","eye",
    "mark","plan","dark","ship","true","blue","deep","best","cold","clear",
    "class","front","note","rain","hold","step","fast","black","white","green",
    "road","short","fire","rock","glass","human","space","warm","girl","ground",
    "reach","direct","team","game","lay","move","ocean","wall","cross","drive",
    "test","record","boat","common","summer","winter","spring","gold","rest","bring",
    "wonder","laugh","check","shape","fall","push","pull","cut","hot","deal",
    "miss","charge","garden","please","round","simple","certain","stay","press","cover",
    "cost","draw","store","figure","hour","week","month","sleep","final","touch",
    "happy","serve","strong","free","break","sense","special","window","join","clean",
    "fresh","quiet","level","matter","ready","above","produce","million","among","list",
    "base","heavy","brown","board","lead","cause","return","corner","love","floor",
    "season","unit","count","machine","moment","length","figure","street","product","dream",
    "natural","course","rise","speak","force","block","stone","table","notice","square",
    "company","usual","appear","train","sign","ball","steel","weather","pass","soft",
    "weight","trade","position","travel","paint","supply","island","market","report","ring",
    "surface","offer","remain","speed","share","total","raise","surprise","wild","dress",
    "captain","piece","energy","silver","broad","safe","except","thick","edge","chance",
    "sight","crowd","exact","chair","danger","station","branch","modern","sugar","camp",
    "promise","master","equal","plain","seat","guard","silent","tail","divide","favor",
    "anger","claim","score","scale","cloud","design","engine","finger","valley","coast",
    "bridge","prize","desert","signal","manage","single","enjoy","price","spread","double"
};
#define WORD_COUNT 490

// ─── Adaptive: words containing specific characters ───
// Finds words from the full dictionary that contain a target character
static inline int word_contains_char(const char* word, char c) {
    for (int i = 0; word[i] != '\0'; i++) {
        if (word[i] == c) return 1;
    }
    return 0;
}

#endif

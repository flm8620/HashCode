

// Forme du score

int scorefinal(vector<int> temps) {
    int res = 0
    for (auto it : temps)
	res += T - (*it);
    return res;
}
	

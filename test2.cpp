
int Calculemap() {

	map<int, vector<pair<int, int> > > mapWareCustProd;

	vector<int> custmerOrder; // given
	map<int, vector<int> > mapCustProd; // given
	map<int, vector<int> > mapWareProd; // given
	vector<int> wares; // given

	for (vector<int>::iterator it = custmerOrder.begin();
			it != custmerOrder.end(); ++it) {
		vector<int> tp = mapCustProd[*it];
		for (vector<int>::iterator it2 = tp.begin(); it2 != tp.end(); ++it2) {
			int wareid = 0;
			float x_before = 100000000000;
			float x_after = 100000000000;
			for (vector<int>::iterator it3 = wares.begin(); it3 != wares.end();
					++it3) {
				vector<int>::iterator target = find(mapWareProd[*it3].begin(),
						mapWareProd[*it3].end(), *it2);
				if (target != mapWareProd[*it3].end()) {
					x_after = 10; // to be changed
					if (x_after < x_before) {
						wareid = *it3;
						x_before = x_after;
					}
				}
			}
			mapWareCustProd[wareid] = pair<int, int>(*it, *it2);
		}
	}
	return 0;
}


struct Drone {
    int x, y;
    vector<>
    vector<>
    int compteur;
};

vector<Drone> drones;

void SecondAlgo() {
    vector<Drone> DroneDispo;
    for(int i = 0; i<T; i++) {
	update(drones)
	Calculemap();
	sort(commands, );
	CalculeObjetRestant

// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


import QtQuick 2.0
import QtTest 1.11
import SortFilterProxyModel 0.2


TestCase {
    name: ""
    when: windowShown


    SortFilterProxyModel {
        id: passthrough
        sourceModel: collections
    }

    SortFilterProxyModel {
        id: collectionPositive
        sourceModel: collections
        filters: ValueFilter {
            roleName: "name"
            value: "test"
        }
    }

    SortFilterProxyModel {
        id: collectionNegative
        sourceModel: collections
        filters: ValueFilter {
            roleName: "name"
            value: "dummy"
        }
    }

    SortFilterProxyModel {
        id: gameFind
        sourceModel: collections.get(0).games
        filters: ValueFilter {
            roleName: "title"
            value: "bbb"
        }
    }

    SortFilterProxyModel {
        id: gameSort
        sourceModel: collections.get(0).games
        sorters: StringSorter {
            roleName: "title"
        }
    }


    function test_passthrough() {
        compare(passthrough.count, 1);
        compare(passthrough.get(0).name, "test");
    }

    function test_collFindPositive() {
        compare(collectionPositive.count, 1);
        compare(collectionPositive.get(0).name, "test");
    }

    function test_collFindNegative() {
        compare(collectionNegative.count, 0);
    }

    function test_gameFind() {
        compare(collections.get(0).games.count, 3);
        compare(gameFind.count, 1);
        compare(gameFind.get(0).title, "bbb");
    }

    function test_gameSort() {
        compare(collections.get(0).games.count, 3);
        compare(gameSort.count, 3);
        compare(gameSort.get(0).title, "aaa");
        compare(gameSort.get(1).title, "bbb");
        compare(gameSort.get(2).title, "ccc");
    }
}

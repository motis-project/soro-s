<template>
    <div>
        <div class="station-search">
            <v-text-field
                :disabled="!currentInfrastructure"
                label="Search for station or halt by name:"
                :error-messages="currentSearchError"
                hide-details="auto"
                @change="updateQuery"
                @keydown.enter.prevent="updateQueryAndSearch"
            />

            <soro-button
                label="Search"
                class="search-button"
                @click="searchName"
            />
        </div>

        <v-list
            v-if="currentSearchedMapPositions.length > 1"
            density="compact"
            elevation="5"
        >
            <v-list-subheader>SEARCH RESULTS</v-list-subheader>
            <v-list-item
                v-for="(mapPosition) in currentSearchedMapPositions"
                :key="mapPosition.name"
                :value="mapPosition.name"
                @click="setCurrentSearchedMapPosition(mapPosition.position)"
            >
                <v-list-item-title>
                    {{ getSearchResultLabelParts(mapPosition.name).before }}<strong class="search-match">
                        {{ currentSearchTerm }}
                    </strong>{{ getSearchResultLabelParts(mapPosition.name).after }}
                </v-list-item-title>
            </v-list-item>
        </v-list>
    </div>
</template>

<script setup lang="ts">
import SoroButton from '@/components/soro-button.vue';
</script>

<script lang="ts">
import { defineComponent } from 'vue';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import { mapActions, mapMutations, mapState } from 'vuex';

const minSearchQueryLimit = 3;

export default defineComponent({
    name: 'StationSearch',

    data(): { currentQuery: string | null } {
        return {
            currentQuery: null,
        };
    },

    computed: {
        ...mapState(InfrastructureNamespace, [
            'currentInfrastructure',
            'currentSearchTerm',
            'currentSearchError',
            'currentSearchedMapPositions',
        ]),
    },

    methods: {
        updateQuery(event: { target: HTMLInputElement }) {
            this.currentQuery = event.target?.value;
        },

        updateQueryAndSearch(event: never) {
            this.updateQuery(event);
            this.searchName();
        },

        searchName() {
            if (!this.currentQuery || this.currentQuery.length < minSearchQueryLimit) {
                return;
            }

            this.searchPositionFromName(this.currentQuery);
        },

        getSearchResultLabelParts(searchResult: string): { before: string, after: string } {
            const beginIndex = searchResult.indexOf(this.currentSearchTerm);

            return {
                before: searchResult.substring(0, beginIndex),
                after: searchResult.substring(beginIndex + this.currentSearchTerm.length),
            };
        },

        ...mapMutations(InfrastructureNamespace, ['setCurrentSearchedMapPosition']),
        ...mapActions(InfrastructureNamespace, ['searchPositionFromName']),
    },
});
</script>

<style scoped>
.station-search {
    display: flex;
    margin-bottom: 10px;
}

.search-button {
    margin-left: 10px;
    height: auto;
}

.search-match {
    color: rgb(var(--v-theme-primary));
}
</style>
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
                    {{ mapPosition.name }}
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

export default defineComponent({
    name: 'StationSearch',

    data(): { currentQuery: string | null } {
        return {
            currentQuery: null
        };
    },

    computed: {
        ...mapState(InfrastructureNamespace, [
            'currentInfrastructure',
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
            if (!this.currentQuery) {
                return;
            }

            console.log(`Will now try searching for ${this.currentQuery}`);
            this.searchPositionFromName(this.currentQuery);
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
</style>
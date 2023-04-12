<template>
  <div>
    <div v-if="showExtendedLink" class="search-extended-link">
      <a href="/" @click="onClickExtended"> Go to extended Search </a>
    </div>

    <div class="search">
      <v-text-field
        ref="searchTextField"
        v-model="currentQuery"
        :disabled="!currentInfrastructure"
        label="Search:"
        :error-messages="currentSearchError"
        hide-details="auto"
        @keydown.enter.prevent="pickFirstResult"
      />

      <soro-button
        label="Search"
        class="search-button"
        @click="pickFirstResult"
      />
    </div>

    <v-list
      v-if="currentSearchResults.length > 1"
      density="compact"
      elevation="5"
      class="search-result-list"
    >
      <v-list-subheader>SEARCH RESULTS</v-list-subheader>
      <v-list-item
        v-for="(searchResult, index) in currentSearchResults"
        :key="index"
        :value="searchResult.name"
        @click="setCurrentBoundingBox(searchResult.boundingBox)"
      >
        <v-list-item-title>
          <img
            class="search-search-type-icon"
            :src="iconSrc(searchResult)"
            alt=""
          />
          <span>{{ highlightedName(searchResult.name).before }}</span>
          <strong class="search-match">{{
            highlightedName(searchResult.name).strong
          }}</strong>
          <span>{{ highlightedName(searchResult.name).after }}</span>
        </v-list-item-title>
      </v-list-item>
    </v-list>
  </div>
</template>

<script setup lang="ts">
import SoroButton from '@/components/base/soro-button.vue';
</script>

<script lang="ts">
import { defineComponent } from 'vue';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import { mapMutations, mapState } from 'vuex';
import { ElementType } from '@/components/infrastructure/element-types';
import { SearchResult, SearchResultType } from '@/util/SoroClient';
import { getIconUrl } from '@/components/infrastructure/add-icons';

const validSearchTypes = [
  ElementType.STATION,
  ElementType.HALT,
  ElementType.MAIN_SIGNAL
];

export default defineComponent({
  name: 'ExtendedSearch',

  props: {
    showExtendedLink: {
      type: Boolean,
      required: false,
      default: false
    },
    showExtendedOptions: {
      type: Boolean,
      required: false,
      default: false
    }
  },

  emits: ['change-to-extended'],

  data(): {
    currentQuery: string;
    validSearchTypes: typeof validSearchTypes;
    currentSearchTypes: string[];
  } {
    return {
      currentQuery: '',
      validSearchTypes,
      currentSearchTypes: [ElementType.STATION, ElementType.HALT]
    };
  },

  computed: {
    ...mapState(InfrastructureNamespace, [
      'currentInfrastructure',
      'currentSearchResults',
      'currentSearchError',
      'currentBoundingBox'
    ])
  },

  watch: {
    async currentQuery(newText: string) {
      this.setCurrentSearchResults(await this.search(newText));
    }
  },

  methods: {
    ...mapMutations(InfrastructureNamespace, [
      'setCurrentQuery',
      'setCurrentSearchResults',
      'setCurrentBoundingBox'
    ]),

    async search(query: string): Promise<SearchResult[]> {
      // server only gives results for queries with at least 3 characters
      if (query.length < 3) {
        return [];
      }

      return this.$store.state.soroClient
        .infrastructure(this.currentInfrastructure)
        .search(query);
    },

    async pickFirstResult() {
      if (this.currentSearchResults?.length === 0) {
        return;
      }

      this.setCurrentBoundingBox(this.currentSearchResults[0].boundingBox);
    },

    highlightedName(searchResultName: string): {
      before: string;
      strong: string;
      after: string;
    } {
      const lowerQuery = this.currentQuery.toLowerCase();
      const lowerName = searchResultName.toLowerCase();

      const beginIndex = lowerName.indexOf(lowerQuery);

      return {
        before: searchResultName.substring(0, beginIndex),
        strong: searchResultName.substring(
          beginIndex,
          beginIndex + this.currentQuery.length
        ),
        after: searchResultName.substring(beginIndex + this.currentQuery.length)
      };
    },

    iconSrc(searchResult: SearchResult): string {
      return getIconUrl(
        searchResult.type === SearchResultType.Element
          ? searchResult.elementType
          : searchResult.type
      );
    },

    onClickExtended(event: Event) {
      event.preventDefault();

      this.$emit('change-to-extended');
    }
  }
});
</script>

<style scoped>
.search {
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

.search-extended-link {
  display: flex;
  color: rgb(var(--v-theme-primary));
  justify-content: right;
}

a,
a:visited,
a:hover,
a:active {
  color: inherit;
}

.search-result-list.v-list {
  overflow-y: auto;
  max-height: 500px;
}

.search-search-type-icon {
  margin-left: 5px;
  margin-right: 5px;
  height: 1.2em;
}
</style>

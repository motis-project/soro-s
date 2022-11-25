import { getFileContents } from "../../../utl/getFileContents.js";

export class TimetableComponent {
  constructor(container) {
    this.container = container;
    this.rootElement = container.element;

    getFileContents("./components/timetable/timetable_component.html")
      .then((html) => {
        this.rootElement.innerHTML = html;
      });
  }
}

document.getElementById("editButton").addEventListener("click", showEditModal);
function showEditModal() {
  // Set the current name in the input field
  const currentName = editButton.getAttribute("data-name");
  document.getElementById('editedName').value = currentName;
  sessionStorage.setItem("currentName", currentName);
  // Show the modal
  $('#editModal').modal('show');
}
document.getElementById("closeModalButton").addEventListener("click", closeEditModal);
function closeEditModal() {
  // Close the modal without saving
  $('#editModal').modal('hide');
}
document.getElementById("saveEditButton").addEventListener("click", saveEditedName);
function saveEditedName() {
  const currentName = sessionStorage.getItem("currentName");
  var editedName = document.getElementById('editedName').value;
    const data = {
    currentName: currentName,
    editedName: editedName
  };

  const headers = new Headers();
  headers.append("Content-Type", "application/json");

  // Make a fetch request with the appropriate data and include credentials
  fetch("/change", {
      method: "POST",
      headers: headers,
      credentials: 'include',
      body: JSON.stringify(data)
  })
  .then(response => {
      if (response.ok) {
          window.location.replace("https://todoapp-a7a9f5a0e861.herokuapp.com/dashboard");
      } else {
          // Handle errors, for example, if the item couldn't be changed
          alert("Failed to change item");
      }
  })
  .catch(error => {
      console.error("Error:", error);
  });
  // Close the modal after saving
  $('#editModal').modal('hide');
}

$(document).ready(function () {
    $('.ui.dropdown').dropdown();
});

function filterTasks(status) {
    const tasks = document.querySelectorAll('.task');
    tasks.forEach(task => {
        const completed = task.classList.contains('completed');
        if (status === 'all' || (status === 'completed' && completed) || (status === 'notCompleted' && !completed)) {
            task.style.display = 'block';
        } else {
            task.style.display = 'none';
        }
    });
}

const filterAllButton = document.getElementById("filterAll");
const filterCompletedButton = document.getElementById("filterCompleted");
const filterNotCompletedButton = document.getElementById("filterNotCompleted");

// Add click event listeners to the filter buttons
filterAllButton.addEventListener("click", function() {
    filterTasks('all');
});

filterCompletedButton.addEventListener("click", function() {
    filterTasks('completed');
});

filterNotCompletedButton.addEventListener("click", function() {
    filterTasks('notCompleted');
});